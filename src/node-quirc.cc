/*
 * node-quirc.cc - glue for Node.js
 */

#include <nan.h>

extern "C" {
	#include "node_quirc_decode.h"
}

using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::CopyBuffer;
using Nan::Error;
using Nan::GetFunction;
using Nan::New;
using Nan::Null;
using Nan::Set;
using Nan::ThrowError;
using Nan::ThrowTypeError;

/* async worker wrapper around nq_decode() */
class NodeQuircDecoder: public AsyncWorker
{
	public:

	/* ctor */
	NodeQuircDecoder(Callback *callback, const uint8_t *img, size_t img_len, size_t width, size_t height):
	    AsyncWorker(callback),
	    m_img(img),
	    m_img_len(img_len),
	    m_width(width),
	    m_height(height),
	    m_code_list(NULL)
	{ }


	/* dtor */
	~NodeQuircDecoder()
	{
		nq_code_list_free(m_code_list);
	}


	// Executed inside the worker-thread.
	// It is not safe to access V8, or V8 data structures here, so
	// everything we need for input and output should go on `this`.
	void Execute()
	{
		m_code_list = nq_decode(m_img, m_img_len, m_width, m_height);
	}


	// Executed when the async work is complete this function will be run
	// inside the main event loop so it is safe to use V8 again
	void HandleOKCallback()
	{
		/* ENOMEM check */
		if (m_code_list == NULL)
			return ThrowError("Could not allocate memory");

		/* global error check */
		if (nq_code_list_err(m_code_list) != NULL)
			return CallbackError(nq_code_list_err(m_code_list));

		unsigned int count = nq_code_list_size(m_code_list);
		v8::Local<v8::Array> results = New<v8::Array>();
		for (unsigned int i = 0; i < count; i++) {
			const struct nq_code *code = nq_code_at(m_code_list, i);
			Nan::Maybe<bool> success = Set(results, i, CodeToObject(code));
			if (success.IsNothing() || !success.FromJust())
				return CallbackError("Set() failed");
		}

		// all went well
		v8::Local<v8::Value> argv[] = {
			Null(), /* err */
			results,
		};
		Nan::Call(*callback, 2, argv);
	}


	private:

	/* members */

	/* nq_decode() arguments */
	const uint8_t	*m_img;
	size_t		 m_img_len;
	size_t		 m_width;
	size_t		 m_height;
	/* nq_decode() return value */
	struct nq_code_list	*m_code_list;

	/* helpers */

	// call `callback` with an Error containing msg.
	void CallbackError(const char *msg)
	{
		v8::Local<v8::Value> argv[] = {
			Error(msg),
		};
		Nan::Call(*callback, 1, argv);
	}


	// "convert" a struct nq_code to a v8::Object
	v8::Local<v8::Object> CodeToObject(const struct nq_code *code)
	{
		v8::Local<v8::Object> obj = New<v8::Object>();
		if (nq_code_err(code) != NULL) {
			Set(obj, New("err").ToLocalChecked(),
			    New(nq_code_err(code)).ToLocalChecked());
		} else {
			Set(obj, New("version").ToLocalChecked(),
			    New(nq_code_version(code)));
			Set(obj, New("ecc_level").ToLocalChecked(),
			    New(nq_code_ecc_level_str(code)).ToLocalChecked());
			Set(obj, New("mask").ToLocalChecked(),
			    New(nq_code_mask(code)));
			Set(obj, New("mode").ToLocalChecked(),
			    New(nq_code_mode_str(code)).ToLocalChecked());
			const char *eci = nq_code_eci_str(code);
			if (eci) {
				Set(obj, New("eci").ToLocalChecked(),
				    New(eci).ToLocalChecked());
			}
			const char *data = (const char *)nq_code_payload(code);
			Set(obj, New("data").ToLocalChecked(),
			   CopyBuffer(data, nq_code_payload_len(code)).ToLocalChecked());
		}
		return (obj);
	}
};


// async access to nq_decode()
NAN_METHOD(NodeQuircDecodeEncodedAsync) {
	if (info.Length() < 2)
		return ThrowError("expected (img, callback) as arguments");
	if (!node::Buffer::HasInstance(info[0]))
		return ThrowTypeError("img must be a Buffer");
	if (!info[1]->IsFunction())
		return ThrowTypeError("callback must be a function");

	uint8_t *img   = (uint8_t *)node::Buffer::Data(info[0]);
	size_t img_len = node::Buffer::Length(info[0]);
	Callback *callback = new Callback(info[1].As<v8::Function>());
	AsyncQueueWorker(new NodeQuircDecoder(callback, img, img_len, 0, 0));
}

NAN_METHOD(NodeQuircDecodeRawAsync) {
	if (info.Length() < 4)
		return ThrowError("expected (pixels, width, height, callback) as arguments");
	if (!info[0]->IsUint8ClampedArray() && !node::Buffer::HasInstance(info[0]))
		return ThrowTypeError("pixels must be a Uint8ClampedArray or Buffer");
	if (!info[1]->IsNumber())
		return ThrowTypeError("width must be a number");
	if (!info[2]->IsNumber())
		return ThrowTypeError("height must be a number");
	if (!info[3]->IsFunction())
		return ThrowTypeError("callback must be a function");

	uint8_t *img;
	size_t img_len;

	if (node::Buffer::HasInstance(info[0])) {
		img = (uint8_t *)node::Buffer::Data(info[0]);
		img_len = node::Buffer::Length(info[0]);
	} else {
		Nan::TypedArrayContents<uint8_t> data(info[0]);
		img = *data;
		img_len = data.length();
	}

	size_t width = (size_t)Nan::To<int>(info[1]).FromJust();
	size_t height = (size_t)Nan::To<int>(info[2]).FromJust();
	Callback *callback = new Callback(info[3].As<v8::Function>());
	AsyncQueueWorker(new NodeQuircDecoder(callback, img, img_len, width, height));
}

// export stuff to NodeJS
NAN_MODULE_INIT(NodeQuircInit) {
	Set(target, New("decodeEncoded").ToLocalChecked(),
	    GetFunction(New<v8::FunctionTemplate>(NodeQuircDecodeEncodedAsync)).ToLocalChecked());
	Set(target, New("decodeRaw").ToLocalChecked(),
	    GetFunction(New<v8::FunctionTemplate>(NodeQuircDecodeRawAsync)).ToLocalChecked());
}


NODE_MODULE(node_quirc, NodeQuircInit)
