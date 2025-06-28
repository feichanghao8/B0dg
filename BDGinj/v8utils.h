#pragma once
#include "external_data/object.h"


//#define OPEN_HANDLE_LIST(V)                                    \
//  V(Template, TemplateInfo)                                    \
//  V(FunctionTemplate, FunctionTemplateInfo)                    \
//  V(ObjectTemplate, ObjectTemplateInfo)                        \
//  V(DictionaryTemplate, DictionaryTemplateInfo)                \
//  V(Signature, FunctionTemplateInfo)                           \
//  V(Data, Object)                                              \
//  V(Number, Number)                                            \
//  V(RegExp, JSRegExp)                                          \
//  V(Object, JSReceiver)                                        \
//  V(Array, JSArray)                                            \
//  V(Map, JSMap)                                                \
//  V(Set, JSSet)                                                \
//  V(ArrayBuffer, JSArrayBuffer)                                \
//  V(ArrayBufferView, JSArrayBufferView)                        \
//  V(TypedArray, JSTypedArray)                                  \
//  V(Uint8Array, JSTypedArray)                                  \
//  V(Uint8ClampedArray, JSTypedArray)                           \
//  V(Int8Array, JSTypedArray)                                   \
//  V(Uint16Array, JSTypedArray)                                 \
//  V(Int16Array, JSTypedArray)                                  \
//  V(Uint32Array, JSTypedArray)                                 \
//  V(Int32Array, JSTypedArray)                                  \
//  V(Float16Array, JSTypedArray)                                \
//  V(Float32Array, JSTypedArray)                                \
//  V(Float64Array, JSTypedArray)                                \
//  V(DataView, JSDataViewOrRabGsabDataView)                     \
//  V(SharedArrayBuffer, JSArrayBuffer)                          \
//  V(Name, Name)                                                \
//  V(String, String)                                            \
//  V(Symbol, Symbol)                                            \
//  V(Script, JSFunction)                                        \
//  V(UnboundModuleScript, SharedFunctionInfo)                   \
//  V(UnboundScript, SharedFunctionInfo)                         \
//  V(Module, Module)                                            \
//  V(Function, JSReceiver)                                      \
//  V(CompileHintsCollector, Script)                             \
//  V(Message, JSMessageObject)                                  \
//  V(Context, NativeContext)                                    \
//  V(External, Object)                                          \
//  V(StackTrace, StackTraceInfo)                                \
//  V(StackFrame, StackFrameInfo)                                \
//  V(Proxy, JSProxy)                                            \
//  V(debug::GeneratorObject, JSGeneratorObject)                 \
//  V(debug::ScriptSource, HeapObject)                           \
//  V(debug::Script, Script)                                     \
//  V(debug::EphemeronTable, EphemeronHashTable)                 \
//  V(debug::AccessorPair, AccessorPair)                         \
//  V(Promise, JSPromise)                                        \
//  V(Primitive, Object)                                         \
//  V(PrimitiveArray, FixedArray)                                \
//  V(BigInt, BigInt)                                            \
//  V(ScriptOrModule, ScriptOrModule)                            \
//  V(FixedArray, FixedArray)                                    \
//  V(ModuleRequest, ModuleRequest)                              \
//  IF_WASM(V, WasmMemoryMapDescriptor, WasmMemoryMapDescriptor) \
//  IF_WASM(V, WasmMemoryObject, WasmMemoryObject)
//
//
//
//
//#ifdef V8_ENABLE_DIRECT_HANDLE
//
//#define MAKE_OPEN_HANDLE(From, To)                                           \
//  v8::internal::Handle<v8::internal::To> Utils::OpenHandle(                  \
//      const v8::From* that, bool allow_empty_handle) {                       \
//    DCHECK(allow_empty_handle || !v8::internal::ValueHelper::IsEmpty(that)); \
//    DCHECK(v8::internal::ValueHelper::IsEmpty(that) ||                       \
//           Is##To(v8::internal::Tagged<v8::internal::Object>(                \
//               v8::internal::ValueHelper::ValueAsAddress(that))));           \
//    if (v8::internal::ValueHelper::IsEmpty(that)) {                          \
//      return v8::internal::Handle<v8::internal::To>::null();                 \
//    }                                                                        \
//    return v8::internal::Handle<v8::internal::To>(                           \
//        v8::HandleScope::CreateHandleForCurrentIsolate(                      \
//            v8::internal::ValueHelper::ValueAsAddress(that)));               \
//  }                                                                          \
//                                                                             \
//  v8::internal::DirectHandle<v8::internal::To> Utils::OpenDirectHandle(      \
//      const v8::From* that, bool allow_empty_handle) {                       \
//    DCHECK(allow_empty_handle || !v8::internal::ValueHelper::IsEmpty(that)); \
//    DCHECK(v8::internal::ValueHelper::IsEmpty(that) ||                       \
//           Is##To(v8::internal::Tagged<v8::internal::Object>(                \
//               v8::internal::ValueHelper::ValueAsAddress(that))));           \
//    return v8::internal::DirectHandle<v8::internal::To>(                     \
//        v8::internal::ValueHelper::ValueAsAddress(that));                    \
//  }                                                                          \
//                                                                             \
//  v8::internal::IndirectHandle<v8::internal::To> Utils::OpenIndirectHandle(  \
//      const v8::From* that, bool allow_empty_handle) {                       \
//    return Utils::OpenHandle(that, allow_empty_handle);                      \
//  }
//
//#else  // !V8_ENABLE_DIRECT_HANDLE
//
//#define MAKE_OPEN_HANDLE(From, To)                                           \
//  v8::internal::Handle<v8::internal::To> Utils::OpenHandle(                  \
//      const v8::From* that, bool allow_empty_handle) {                       \
//    DCHECK(allow_empty_handle || !v8::internal::ValueHelper::IsEmpty(that)); \
//    DCHECK(v8::internal::ValueHelper::IsEmpty(that) ||                       \
//           Is##To(v8::internal::Tagged<v8::internal::Object>(                \
//               v8::internal::ValueHelper::ValueAsAddress(that))));           \
//    return v8::internal::Handle<v8::internal::To>(                           \
//        reinterpret_cast<v8::internal::Address*>(                            \
//            const_cast<v8::From*>(that)));                                   \
//  }                                                                          \
//                                                                             \
//  v8::internal::DirectHandle<v8::internal::To> Utils::OpenDirectHandle(      \
//      const v8::From* that, bool allow_empty_handle) {                       \
//    return Utils::OpenHandle(that, allow_empty_handle);                      \
//  }                                                                          \
//                                                                             \
//  v8::internal::IndirectHandle<v8::internal::To> Utils::OpenIndirectHandle(  \
//      const v8::From* that, bool allow_empty_handle) {                       \
//    return Utils::OpenHandle(that, allow_empty_handle);                      \
//  }
//
//#endif 


//namespace v8 {
//
//
//
//
//    MaybeLocal<String> Value::ToString(Local<Context> context) const {
//        auto obj = Utils::OpenDirectHandle(this);
//        if (i::IsString(*obj)) return ToApiHandle<String>(obj);
//        PREPARE_FOR_EXECUTION(context, Object, ToString);
//        Local<String> result;
//        has_exception =
//            !ToLocal<String>(i::Object::ToString(i_isolate, obj), &result);
//        RETURN_ON_FAILED_EXECUTION(String);
//        RETURN_ESCAPED(result);
//    }
//
//}