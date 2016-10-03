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
using Nan::MaybeLocal;
using Nan::New;
using Nan::Null;
using Nan::Set;
using Nan::ThrowError;
using Nan::ThrowTypeError;

/* async worker wrapper around node_quirc_decode() */
class NodeQuircDecoder: public AsyncWorker
{
	private:

	const uint8_t *m_img;
	size_t   m_img_len;
	struct nq_code_list *m_code_list;

	public:

	/* ctor */
	NodeQuircDecoder(Callback *callback, const uint8_t *img, size_t img_len):
	    AsyncWorker(callback),
	    m_img(img),
	    m_img_len(img_len),
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
		m_code_list = nq_decode(m_img, m_img_len);
	}

	// Executed when the async work is complete this function will be run
	// inside the main event loop so it is safe to use V8 again
	void HandleOKCallback () {
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
		callback->Call(2, argv);
	}

	void CallbackError(const char *msg) {
		v8::Local<v8::Value> argv[] = {
			Error(msg),
		};
		callback->Call(1, argv);
	}

	v8::Local<v8::Object> CodeToObject(const struct nq_code *code) {
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
			Set(obj, New("data_type").ToLocalChecked(),
			    New(nq_code_data_type_str(code)).ToLocalChecked());
			const char *data = (const char *)nq_code_payload(code);
			Set(obj, New("data").ToLocalChecked(),
			   Nan::CopyBuffer(data, nq_code_payload_len(code)).ToLocalChecked());
		}
		return (obj);
	}
};

// Asynchronous access to the `node_quirc_decode()` function
NAN_METHOD(NodeQuircDecodeAsync) {
	if (info.Length() < 2)
		ThrowError("expected (img, callback) as arguments");
	if (!node::Buffer::HasInstance(info[0]))
		ThrowTypeError("img must be a Buffer");
	if (!info[1]->IsFunction())
		ThrowTypeError("callback must be a Buffer");

	char *img      = node::Buffer::Data(info[0]);
	size_t img_len = node::Buffer::Length(info[0]);
	Callback *callback = new Callback(info[1].As<v8::Function>());
	AsyncQueueWorker(new NodeQuircDecoder(callback, (uint8_t *)img, img_len));
}

NAN_MODULE_INIT(NodeQuircInit) {
	Set(target, New("decode").ToLocalChecked(),
	    GetFunction(New<v8::FunctionTemplate>(NodeQuircDecodeAsync)).ToLocalChecked());
}

NODE_MODULE(node_quirc, NodeQuircInit)
