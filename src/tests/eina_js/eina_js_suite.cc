
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <v8.h>

#include <cassert>
#include <cstdlib>

#include <Eina.h>
#include <Eina.hh>
#include <Eo.hh>

#include <eina_js_list.hh>

static const char script[] =
  "function assert(condition, message) {\n"
  "  if (!condition) {\n"
  "      print(\"Assertion failed \", message);\n"
  "      throw message || \"Assertion failed\";\n"
  "  }\n"
  "}\n"
  "print(\"teste\");\n"
  "var l1 = raw_list;\n"
  "print (\"l1 \", l1.toString());\n"
  "assert (l1.length == 3)\n;\n"
  "var l2 = raw_list;\n"
  "print (\"l2 \", l2.toString());\n"
  "assert (l2.length == 3)\n;\n"
  "var c = l1.concat(l2);\n"
  "print (\"c \", c.toString());\n"
  "assert (c.length == (l1.length + l2.length));\n"
  "assert (c[0] == l1[0]);\n"
  "assert (c[1] == l1[1]);\n"
  "assert (c[2] == l1[2]);\n"
  "assert (c[3] == l2[0]);\n"
  "assert (c[4] == l2[1]);\n"
  "assert (c[5] == l2[2]);\n"
  "assert (c.indexOf(c[0]) == 0);\n"
  "assert (c.indexOf(c[1]) == 1);\n"
  "assert (c.indexOf(c[2]) == 2);\n"
  "assert (c.indexOf(c[3]) == 0);\n"
  "assert (c.indexOf(c[4]) == 1);\n"
  "assert (c.indexOf(c[5]) == 2);\n"
  "assert (c.lastIndexOf(c[0]) == 3);\n"
  "assert (c.lastIndexOf(c[1]) == 4);\n"
  "assert (c.lastIndexOf(c[2]) == 5);\n"
  "assert (c.lastIndexOf(c[3]) == 3);\n"
  "assert (c.lastIndexOf(c[4]) == 4);\n"
  "assert (c.lastIndexOf(c[5]) == 5);\n"
  "var s1 = l1.slice(1, 3);\n"
  "print (\"s1 \", s1.toString());\n"
  "assert (s1.length == 2);\n"
  "assert (s1[0] == l1[1]);\n"
  "assert (s1[1] == l1[2]);\n"
  "var s2 = c.slice(1, 3);\n"
  "print (\"s2 \", s2.toString());\n"
  "assert (s2.length == 2);\n"
  "assert (s2[0] == l1[1]);\n"
  "assert (s2[1] == l1[2]);\n"
  ;

const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate,
                   v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name)
{
  v8::HandleScope handle_scope(isolate);
  v8::TryCatch try_catch;
  v8::ScriptOrigin origin(name);
  v8::Handle<v8::Script> script = v8::Script::Compile(source, &origin);
  if (script.IsEmpty()) {
    std::abort();
    // Print errors that happened during compilation.
    // if (report_exceptions)
    //   ReportException(isolate, &try_catch);
    return false;
  }
  else
  {
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      std::cout << "Failed with exception thrown" << std::endl;
      assert(try_catch.HasCaught());
      std::abort();
      // Print errors that happened during execution.
      // if (report_exceptions)
      //   ReportException(isolate, &try_catch);
      return false;
    } else {
      assert(!try_catch.HasCaught());
      // if (print_result && !result->IsUndefined()) {
      //   // If all went well and the result wasn't undefined then print
      //   // the returned value.
      //   v8::String::Utf8Value str(result);
      //   const char* cstr = ToCString(str);
      //   printf("%s\n", cstr);
      // }
      return true;
    }
  }
}

void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

EAPI void eina_container_register(v8::Handle<v8::ObjectTemplate> global, v8::Isolate* isolate);
EAPI v8::Handle<v8::FunctionTemplate> get_list_instance_template();

int main(int argc, char* argv[])
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;
  
  v8::V8::InitializeICU();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  v8::Isolate* isolate = v8::Isolate::GetCurrent();

  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Context> context;

  efl::eina::ptr_list<int> list;
  list.push_back(new int(5));
  list.push_back(new int(10));
  list.push_back(new int(15));

  efl::js::range_eina_list<int> raw_list(list.native_handle());
  {
    // Create a template for the global object.
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
    // Bind the global 'print' function to the C++ Print callback.
    global->Set(v8::String::NewFromUtf8(isolate, "print"),
                v8::FunctionTemplate::New(isolate, Print));
    eina_container_register(global, isolate);
    // // Bind the global 'read' function to the C++ Read callback.
    // global->Set(v8::String::NewFromUtf8(isolate, "read"),
    //             v8::FunctionTemplate::New(isolate, Read));
    // // Bind the global 'load' function to the C++ Load callback.
    // global->Set(v8::String::NewFromUtf8(isolate, "load"),
    //             v8::FunctionTemplate::New(isolate, Load));
    // // Bind the 'quit' function
    // global->Set(v8::String::NewFromUtf8(isolate, "quit"),
    //             v8::FunctionTemplate::New(isolate, Quit));
    // // Bind the 'version' function
    // global->Set(v8::String::NewFromUtf8(isolate, "version"),
    //             v8::FunctionTemplate::New(isolate, Version));

    context = v8::Context::New(isolate, NULL, global);
  }
  if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
    return 1;
  }
  context->Enter();
  {
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> name(v8::String::NewFromUtf8(context->GetIsolate(), "(shell)"));

    v8::Handle<v8::Value> a[] = {v8::External::New(isolate, &raw_list)};
    context->Global()->Set(v8::String::NewFromUtf8(isolate, "raw_list")
                , get_list_instance_template()->GetFunction()->NewInstance(1, a));     
    
    v8::HandleScope handle_scope(context->GetIsolate());
    ExecuteString(context->GetIsolate(),
                  v8::String::NewFromUtf8(context->GetIsolate(), script),
                  name);
  }
  context->Exit();
}

