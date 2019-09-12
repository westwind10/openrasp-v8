/*
 * Copyright 2017-2019 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bundle.h"

namespace openrasp_v8 {
using namespace std;
Exception::Exception(v8::Isolate* isolate, v8::TryCatch& try_catch) : string() {
  if (!try_catch.HasCaught()) {
    return;
  }
  string& ref = *this;
  v8::HandleScope handle_scope(isolate);
  v8::String::Utf8Value exception(isolate, try_catch.Exception());
  v8::Local<v8::Message> message = try_catch.Message();
  if (message.IsEmpty()) {
    ref.append(*exception).append("\n");
  } else {
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    std::string filename = "<native code>";
    int linenum = 0;
    if (!message->GetScriptOrigin().ResourceName()->IsUndefined()) {
      filename = *v8::String::Utf8Value(isolate, message->GetScriptOrigin().ResourceName());
      linenum = message->GetLineNumber(context).FromMaybe(0);
    }
    ref.append(filename).append(":").append(to_string(linenum)).append("\n");
    v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
    ref.append(*sourceline).append("\n");
    int start = message->GetStartColumn(context).FromMaybe(-1);
    int end = message->GetEndColumn(context).FromMaybe(-1);
    if (start >= 0 && end >= 0) {
      ref.append(start, ' ').append(end - start, '^').append("\n");
    }
    v8::Local<v8::Value> stack_trace_string;
    if (try_catch.StackTrace(context).ToLocal(&stack_trace_string) && stack_trace_string->IsString() &&
        v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
      v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
      ref.append(*stack_trace).append("\n");
    } else {
      ref.append(*exception).append("\n");
    }
  }
}
}  // namespace openrasp_v8