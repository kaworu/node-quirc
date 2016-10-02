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

/* async worker wrapper around node_quirc_decode() */
class NodeQuircDecoder: public AsyncWorker
{
	private:

	uint8_t *m_img;
	size_t   m_img_len;
	struct nq_code_list *m_codes;

	public:

	/* ctor */
	NodeQuircDecoder(Callback *callback, uint8_t *img, size_t img_len):
	    AsyncWorker(callback),
	    m_img(img),
	    m_img_len(img_len),
	    m_codes(NULL),
	{ }

	/* dtor */
	~NodeQuircDecoder()
	{
		nq_code_list_free(m_codes);
	}

	// Executed inside the worker-thread.
	// It is not safe to access V8, or V8 data structures here, so
	// everything we need for input and output should go on `this`.
	void Execute()
	{
		m_codes = nq_decode(m_img, m_img_len);
	}

	// Executed when the async work is complete this function will be run
	// inside the main event loop so it is safe to use V8 again
	void HandleOKCallback () {
		MaybeLocal<v8::String> maybeString =
		    New<v8::String>((const char *)m_decoded_data, m_decoded_data_len);
		if (maybeString.IsEmpty()) {
			v8::Local<v8::Value> argv[] = {
				Error("something failed"), // FIXME
			};
			callback->Call(1, argv);
		} else {
			v8::Local<v8::Value> argv[] = {
				Null(), /* no error */
				maybeString.ToLocalChecked(),
			};
			callback->Call(2, argv);
		}
	}
};

// Asynchronous access to the `node_quirc_decode()` function
NAN_METHOD(NodeQuircDecodeAsync) {
	if (info.Length() < 2)
		Nan::ThrowError("expected (img, callback) as arguments");
	if (!node::Buffer::HasInstance(info[0]))
		Nan::ThrowTypeError("img must be a Buffer");
	if (!info[1].>IsFunction())
		Nan::ThrowTypeError("callback must be a Buffer");

	char	*img       = node::Buffer::Data(info[0]);
	size_t	*img_len   = node::Buffer::Length(info[0]);
	Callback *callback = new Callback(info[1].As<v8::Function>());
	AsyncQueueWorker(new NodeQuircDecoder((uint8_t *)img, img_len, callback));
}

NAN_MODULE_INIT(NodeQuircInit) {
	Set(target, New("decode").ToLocalChecked(),
	    GetFunction(New<v8::FunctionTemplate>(NodeQuircDecodeAsync)).ToLocalChecked());
}

NODE_MODULE(node_quirc, NodeQuircInit)
