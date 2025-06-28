//#include "src/objects/union.h"


#include "nlohmann/json.hpp"
#include "minhook/MinHook.h"

#include "general.h"
    
#include <cstdarg>

#include "external_data/object.h"
//#include "external_data.h"
//#include "v8utils.h"

using ValueVector = std::vector<v8::Local<v8::Value>>;


#define CONS_STRING_TYPE 33
#define ONE_BYTE_STRING_TYPE 40
#define CONS_ONE_BYTE_STRING_TYPE 41
#define ODDBALL_TYPE 67
#define JS_OBJECT_TYPE 1057
#define JS_API_OBJECT_TYPE 1058
#define JS_ARRAY_TYPE 1200


typedef void* (__thiscall * GetNamePtr)(void* thisptr, void*);
GetNamePtr Function_GetName = 0;


typedef v8::MaybeLocal<v8::String>(__fastcall* ValueToStringPtr)(void* thisptr,
    v8::MaybeLocal<v8::String>* result,
    v8::Local<v8::Context> a4);
ValueToStringPtr Function_ValueToString = 0;


typedef void* (__thiscall *FromV8ValuePtr)(
    void* thisptr,
    void* result,
    v8::Local<v8::Value> a3,
    v8::Local<v8::Context> a4);
FromV8ValuePtr Function_FromV8Value = 0;

typedef v8::MaybeLocal<v8::String>* (__cdecl *Function_StringifyPtr)(
    v8::MaybeLocal<v8::String>* result,
    v8::Local<v8::Context> a2,
    v8::Local<v8::Value> a3,
    v8::Local<v8::String> a4);
Function_StringifyPtr Function_Stringify = 0;
void* PPP = 0;


//void*(__cdecl *detourCallMethodWithArgsOrig)(void* thisptr, void* a, v8::Local<v8::Object> obj, char* method, ValueVector* args);
//void* __cdecl detourCallMethodWithArgs(void* thisptr, void* a, v8::Local<v8::Object> obj, char* method, ValueVector* args)
//{
// 
//
//    obj->ToString();
// 
//    args->data();
//    logfff("---------: %x %s, %d %d\n", method, method, args->size(), args->end() - args->begin());
//
//    return detourCallMethodWithArgsOrig(thisptr, a, obj, method, args);;
//}


v8::Local<v8::Value> (__cdecl* detourCallMethodWithArgsOrig)(v8::Isolate* isolate, v8::Local<v8::Object> obj, char* method, ValueVector* args);
v8::Local<v8::Value> __cdecl detourCallMethodWithArgs(v8::Isolate* isolate, v8::Local<v8::Object> obj, char* method, ValueVector* args)
{

    //obj->ToString();
    //isolate->VisitWeakHandles();

    char* test = *(&method - 0x30);
    //obj->ToString();

    logfff(1, "---------: %x %s, %d %d %x\n", method, method, args->size(), args->end() - args->begin());

    return detourCallMethodWithArgsOrig(isolate, obj, method, args);;
}


//namespace v8{
//Local<Value>  Function::GetName() const {
//    auto self = Utils::OpenDirectHandle(this);
//    i::Isolate* i_isolate = self->GetIsolate();
//    if (i::IsJSBoundFunction(*self)) {
//        auto func = i::Cast<i::JSBoundFunction>(self);
//        i::DirectHandle<i::Object> name;
//        ASSIGN_RETURN_ON_EXCEPTION_VALUE(
//            i_isolate, name, i::JSBoundFunction::GetName(i_isolate, func),
//            Local<Value>());
//        return Utils::ToLocal(name);
//    }
//    if (i::IsJSFunction(*self)) {
//        auto func = i::Cast<i::JSFunction>(self);
//        return Utils::ToLocal(i::direct_handle(func->shared()->Name(), i_isolate));
//    }
//    return ToApiHandle<Primitive>(i_isolate->factory()->undefined_value());
//}
//}


//std::string ExtractString(v8::Local<v8::Value> p, v8::Isolate* isolate) {
//    if (p->IsString()) {
//        v8::Local<v8::String> str = v8::Local<v8::String>::Cast(p);
//        v8::String::Utf8Value utf8(isolate, str);
//        return std::string(*utf8);
//    }
//    return "";  // Return an empty string if the value is not a string
//}


//v8::Isolate* v8::Isolate::GetCurrent() {
//    v8::i::Isolate* i_isolate = v8::i::Isolate::Current();
//    return reinterpret_cast<Isolate*>(i_isolate);
//}

//extern thread_local v8::Isolate* g_current_isolate_;
//v8::internal::Handle<v8::internal::To> Utils::OpenHandle(const v8::From* that, bool allow_empty_handle) {
//    DCHECK(allow_empty_handle || !v8::internal::ValueHelper::IsEmpty(that));
//    DCHECK(v8::internal::ValueHelper::IsEmpty(that) ||
//        Is##To(v8::internal::Tagged<v8::internal::Object>(
//            v8::internal::ValueHelper::ValueAsAddress(that))));
//    if (v8::internal::ValueHelper::IsEmpty(that)) {
//        return v8::internal::Handle<v8::internal::To>::null();
//    }
//    return v8::internal::Handle<v8::internal::To>(
//        v8::HandleScope::CreateHandleForCurrentIsolate(
//            v8::internal::ValueHelper::ValueAsAddress(that)));
//}
//int v8::String::Length() const {
//    return static_cast<int>(v8::Utils::OpenDirectHandle(this)->length());
//}

const char* _getValueTypeStr(v8::Value* value) {
    // NOTE: only basic types supported.
    // Other will require hooking or manual importing of some thread local global var and 
    // other 'stuff'. Maybe do later if need.
    if (value->IsUndefined()) { return "Undefined"; }
    else if (value->IsNull()) { return "Null"; }
    else if (value->IsNullOrUndefined()) { return "NullOrUndefined"; }
    else if (value->IsString()) { return "String"; }
    //else if (value->IsTrue()) { return "True"; }
    else { return "Not supported yet!"; }


}

//namespace v8 {
//    class Utils {
//        //v8::internal::Handle<v8::internal::String>
//        void *
//        Utils::OpenHandle(const v8::String* that, bool allow_empty_handle) {
//            //return v8::internal::Handle<v8::internal::String>(
//            //    reinterpret_cast<v8::internal::Address*>(const_cast<v8::String*>(that)));
//            return v8::internal::Handle<v8::internal::String>(
//                reinterpret_cast<v8::internal::Address*>(const_cast<v8::String*>(that)));
//        }
//
//    }
//
//
//    MaybeLocal<String> Value::ToString(Local<Context> context) const {
//        auto obj = Utils::OpenHandle(this);
//        if (obj->IsString()) return ToApiHandle<String>(obj);
//       
//        //PREPARE_FOR_EXECUTION(context, Object, ToString, String);
//        //Local<String> result;
//        //has_pending_exception =
//        //    !ToLocal<String>(i::Object::ToString(isolate, obj), &result);
//        //RETURN_ON_FAILED_EXECUTION(String);
//        //RETURN_ESCAPED(result);
//    }
//}

int v8::Value::xxx() {
    typedef v8::internal::Object O;
    typedef v8::internal::Internals I;
    O* obj = *reinterpret_cast<O* const*>(this);
    
    return I::GetInstanceType(obj);

    //if (!I::HasHeapObjectTag(obj)) return false;
    //return (I::GetInstanceType(obj) < I::kFirstNonstringType);
}

//Local<String> Value::__test() {
//        return ToString(Isolate::GetCurrent()->GetCurrentContext())
//            .FromMaybe(Local<String>());
//}

//void ReadExternalStringDirectly(v8::Local<v8::Value> value) {
//    logfff(1, "----------------- READ string...\n");
//    if (!value->IsString()) {
//        std::cerr << "Value is not a string!" << std::endl;
//        return;
//    }
//
//    v8::Local<v8::String> str = value.As<v8::String>();
//
//    // Check if the string is external
//    //if (str->IsExternal()) {
//    if (1) {
//        // Use GetExternalStringResource to confirm type (safer)
//        auto resource = str->GetExternalStringResource();
//        if (resource) {
//            const uint16_t* rawData = resource->data();
//            size_t length = resource->length();
//            std::wcout << L"External String Data (Direct): ";
//            for (size_t i = 0; i < length; ++i) {
//                std::wcout << static_cast<wchar_t>(rawData[i]);
//            }
//            std::wcout << std::endl;
//        }
//        else {
//            std::cerr << "External string resource is null." << std::endl;
//        }
//    }
//    else {
//        std::cerr << "String is not external." << std::endl;
//    }
//}

void getTagRecursive() {

}

//template <typename T>
//V8_INLINE static T ReadField(const internal::Object* ptr, int offset) {
//    const uint8_t* addr =
//        reinterpret_cast<const uint8_t*>(ptr) + offset - kHeapObjectTag;
//    return *reinterpret_cast<const T*>(addr);
//}

uint8_t* readField(uint8_t* objPtr, int offset) {
    int HEAP_OBJECT_SIZE = 1;
    uint8_t* address = objPtr + offset - HEAP_OBJECT_SIZE;
    return *(uint8_t**)address;
}

 int getTTtype(uint8_t* obj) {
     int TYPE_INFO_OFFSET = 8;
     
     //logfff(1, "obj: %x\n", obj);
     if (obj == 0) { return -1;  }

     obj = *(uint8_t**)obj;

     //logfff(1, "obj: %x\n", obj);
     if (obj == 0) { return -2;  }
     
     if (!((int)obj & 1)) {
         // NOTE: this is not pointer to map. Something is fishy.
         return -3;
     }

     uint8_t* map = readField(obj, 0);
     //logfff(1, "map: %x\n", map);

     uint16_t result = (uint16_t)readField(map, TYPE_INFO_OFFSET);
     
     //result = result & 0xff;
     result = result & 0xffff;
     return result;

    //return ReadField<uint16_t>(map, kMapInstanceTypeAndBitFieldOffset) & 0xff;
}

 
 bool IsSMI(uint8_t* ptr) {
     int x = (int)ptr;
     bool result = (x & 1) == 0;
     return result;
     //return reinterpret_cast<intptr_t>(value) & 1 == 0; // LSB = 0 for SMIs
 }

 int32_t ExtractSMIValue(uint8_t* ptr) {
     int x = (int)ptr;
     return x >> 1;
     //return static_cast<int32_t>(ptr >> 1);
 }

 //void ReadIntegerFromRawPointer(intptr_t ptr) {
 double readIntegerFromRawPointer(uint8_t* ptr) {
     //intptr_t p = (intptr_t)ptr;
     if (IsSMI(ptr)) {
         int32_t value = ExtractSMIValue(ptr);
         return value;
         //std::cout << "SMI value: " << value << std::endl;
     }
     else {
         //auto* heap_object = reinterpret_cast<void*>(p & ~1);
         //double* double_value = reinterpret_cast<double*>(
         //    reinterpret_cast<uint8_t*>(heap_object) + /* offset to value field */ 8);
         //return *double_value;
         //std::cout << "Heap Number value: " << *double_value << std::endl;

         uint8_t* heapObj = (uint8_t*)(((int)ptr) & ~1);
         double* result = (double*)(heapObj + 8);
         return *result;
     }
 }

 void readConsString(uint8_t* p) {
     //logfff(1, "p: %x\n", p);

     uint8_t* left = *(uint8_t**)(p + 12);
     //logfff(1, "left: %x\n", left);

     uint8_t* right = *(uint8_t**)(p + 16);
     //logfff(1, "right: %x\n", right);

     int lt = getTTtype(left);
     //logfff(1, "lt: %x\n", lt);
     int rt = getTTtype(right);
     //logfff(1, "rt: %x%\n", rt);
 }

 struct loc_buff {
     char* data;
     int size;
 };
 loc_buff readFlatString(uint8_t* p) {
     p = p - 1;
     int size = *(int*)(p + 8);
     char* string = (char*)(p + 12);
     loc_buff buf = {
         .data = string,
         .size = size,
     };
     //logfff(1, "Flat String: %d: %.*s\n", size, size, string);
     return buf;
 }

 const char* getOddBallType(v8::Value* value) {
     if (value->IsNull()) { return "v: NULL"; }
     else if (value->IsUndefined()) { return "v: UNDEFINED"; }
     // TODO: Get directly 
     //else if (value->IsBoolean()) { 
     //    return "v: Bool"; 
     //}
     else { return "ERROR: wrong oddball"; }
 }

 loc_buff testReadFlatString(uint8_t* p) {
     loc_buff error = {.data = (char*)"", .size = 1 };
     if (p == 0) { return error; }
     
     char* a1 = *(char**)p;
     if (a1 == 0) { return error; }

     char* a2 = *(char**)a1;
     if (a2 == 0) { return error; }

     // TODO: Test for SMI and do proper processing
     char* obj = a2 - 1;

     int size = *(int*)(obj + 8);
     char* string = obj + 12;

     if (*string == 0) { return error; }
     loc_buff result = {
         .data = string,
         .size = size
     };
     return result;
 }



 bool isValidUTF8(const char* str, size_t length) {
     size_t i = 0;
     while (i < length) {
         unsigned char c = static_cast<unsigned char>(str[i]);

         if (c <= 0x7F) {
             i++;
         }
         else if ((c >= 0xC0 && c <= 0xDF) && i + 1 < length && (static_cast<unsigned char>(str[i + 1]) & 0xC0) == 0x80) {
             i += 2;
         }
         else if ((c >= 0xE0 && c <= 0xEF) && i + 2 < length && (static_cast<unsigned char>(str[i + 1]) & 0xC0) == 0x80 && (static_cast<unsigned char>(str[i + 2]) & 0xC0) == 0x80) {
             i += 3;
         }
         else if ((c >= 0xF0 && c <= 0xF7) && i + 3 < length && (static_cast<unsigned char>(str[i + 1]) & 0xC0) == 0x80 && (static_cast<unsigned char>(str[i + 2]) & 0xC0) == 0x80 && (static_cast<unsigned char>(str[i + 3]) & 0xC0) == 0x80) {
             i += 4;
         }
         else {
             return false;
         }
     }
     return true;
 }

 bool isValidUTF16(const char* str, size_t length) {
     if (length >= 2) {
         uint16_t bom = (static_cast<unsigned char>(str[0]) << 8) | static_cast<unsigned char>(str[1]);
         if (bom == 0xFEFF || bom == 0xFFFE) {
             return true;
         }
     }

     size_t i = 0;
     while (i < length) {
         uint16_t code_unit = (static_cast<unsigned char>(str[i]) << 8) | static_cast<unsigned char>(str[i + 1]);
         i += 2;

         if ((code_unit >= 0xD800 && code_unit <= 0xDFFF)) {
             return true;
         }
         if ((code_unit >= 0x0000 && code_unit <= 0xD7FF) || (code_unit >= 0xE000 && code_unit <= 0xFFFF)) {
             continue;
         }
         else {
             return false;
         }
     }

     return true;
 }



 char* Utf16ToUtf8(const char* utf16_str, size_t length) {
     // Estimate the size of the UTF-8 string (maximum size)
     size_t utf8_length = length * 3; // Worst case: 3 bytes per UTF-16 character (e.g., for 3-byte UTF-8 characters)

     // Allocate memory for the UTF-8 string
     char* utf8_str = new char[utf8_length + 1]; // +1 for null terminator

     size_t i = 0;
     size_t j = 0;

     while (i < length) {
         // Read a UTF-16 character (2 bytes at a time)
         uint16_t utf16_char = (static_cast<uint8_t>(utf16_str[i]) << 8) | static_cast<uint8_t>(utf16_str[i + 1]);
         i += 2;

         if (utf16_char <= 0x7F) {
             // 1 byte UTF-8 character (ASCII)
             utf8_str[j++] = static_cast<char>(utf16_char);
         }
         else if (utf16_char <= 0x7FF) {
             // 2 byte UTF-8 character
             utf8_str[j++] = 0xC0 | (utf16_char >> 6);
             utf8_str[j++] = 0x80 | (utf16_char & 0x3F);
         }
         else if (utf16_char >= 0xD800 && utf16_char <= 0xDBFF) {
             uint16_t high_surrogate = utf16_char;
             uint16_t low_surrogate = (static_cast<uint8_t>(utf16_str[i]) << 8) | static_cast<uint8_t>(utf16_str[i + 1]);
             i += 2; // Move past low surrogate

             uint32_t code_point = ((high_surrogate - 0xD800) << 10) + (low_surrogate - 0xDC00) + 0x10000;

             // Convert to UTF-8
             utf8_str[j++] = 0xF0 | (code_point >> 18);
             utf8_str[j++] = 0x80 | ((code_point >> 12) & 0x3F);
             utf8_str[j++] = 0x80 | ((code_point >> 6) & 0x3F);
             utf8_str[j++] = 0x80 | (code_point & 0x3F);
         }
         else {
             //std::cerr << "Invalid UTF-16 character: " << std::hex << utf16_char << std::endl;
             logfff(2, "Invalid utf16 character %x\n", utf16_char);
             delete[] utf8_str;
             break;
             //throw std::runtime_error("Invalid UTF-16 character encountered");
         }
     }

     // Null-terminate the UTF-8 string
     if (utf8_str) {
         utf8_str[j] = '\0';
     }


     return utf8_str;
 }




 loc_buff testReadFlatString1(uint8_t* p) {
     loc_buff error = { .data = (char*)"", .size = 1 };
     if (p == 0) { return error; }

     char* a1 = *(char**)p;
     if (a1 == 0) { return error; }


     // TODO: Test for SMI and do proper processing
     char* obj = a1 - 1;

     int size = *(int*)(obj + 8);
     char* string = obj + 12;

     if (*string == 0) { return error; }
     loc_buff result = {
         .data = string,
         .size = size
     };
     return result;

 }


 void testPrintWithoutNewLines(char* data, int size) {
     bool isUtf8 = isValidUTF8(data, size);
     if (isUtf8) {
         //logfff(1, "-------------:   8 [%d]%.*s\n", size, size, data);
         logfff(1, "%.*s", size, data);
     }
     else {
         //logfff(1, "-......................: %d %x\n", size, data);
         char* utf16_str = Utf16ToUtf8(data, size);
         if (utf16_str && *utf16_str != 0) {
             //logfff(1, "-------------: 16 [%d] %s\n", size, utf16_str);
             logfff(1, "%s", size, utf16_str);
             //delete[] utf16_str;
         }
         else {
         }
     }
 }

loc_buff testReadConsString(uint8_t * p) {
    loc_buff error = { .data = (char*)"", .size = 1 };
    
    //logfff(1, "= %x\n", p);
    if (p == 0) { return error; }

    uint8_t* a1 = *(uint8_t**)p;
    //logfff(1, "= %x\n", a1);
    if (a1 == 0) { return error; }

    //uint8_t* a2 = *(uint8_t**)a1;
    ////logfff(1, "= %x\n", a2);
    //if (a2 == 0) { return error; }

    uint8_t* obj = a1 - 1;
    //logfff(1, "= %x\n", obj);

    uint8_t* left = obj + 12;
    uint8_t* right = obj + 16;
    //logfff(1, "= %x\n", left);
    //logfff(1, "= %x\n", right);

    // TODO: First check that both sides are present and correct string

    //logfff(1, "about to read left and right");

    int leftType = getTTtype(left);
    int rightType = getTTtype(right);

    //logfff(1, "left type: %d\n", leftType);
    //logfff(1, "right type: %d\n", rightType);

    if (leftType == CONS_ONE_BYTE_STRING_TYPE) {
        testReadConsString(left);
    }
    else if (leftType == ONE_BYTE_STRING_TYPE) {
        loc_buff l = testReadFlatString1(left);
        testPrintWithoutNewLines(l.data, l.size);
    }


    if (rightType == CONS_ONE_BYTE_STRING_TYPE) {
        testReadConsString(right);
    }
    else if (rightType == ONE_BYTE_STRING_TYPE) {
        loc_buff r = testReadFlatString1(right);
        testPrintWithoutNewLines(r.data, r.size);
    }

    //loc_buff l = testReadFlatString1(left);
    //loc_buff r = testReadFlatString1(right);


 /*   testPrint(l.data, l.size);
    testPrint(r.data, r.size);*/

    return error;
}
 

char* getFunctionName(void* function) {
   try {
       static int count = 0;
       v8::Local<v8::Value> result2 = {};
       //void* result = malloc(1024*8);
       //if (count++ > 1000) {
           int z = 0;
           auto x = Function_GetName(function, &result2);
           auto b = *result2;
           v8::Local<v8::String> str = v8::Local<v8::String>::Cast(result2);

           char* p = **(char***)x;
           int size1 = *(int*)(p + 7);
           char* text1 = p + 7 + 4;

           return text1;

           logfff(1, "------------------ Function: %.*s\n", size1, text1);

           p = **((char***)x + 5); // 5*4
           int size2 = *(int*)(p + 7);
           char* text2 = p + 7 + 4;


   }
   catch (...) {
       logfff(1, "Exception ...\n");
   }
}


void*(__fastcall * detourFunctionCallOrig)(void*b1, void*b2, void* a1, v8::Local<v8::Context> a2, void*a3, int argc, v8::Local<v8::Value> argv[]);
void* __fastcall  detourFunctionCall(void* b1, void* b2, void* a1, v8::Local<v8::Context> a2, void* a3, int argc, v8::Local<v8::Value> argv[])
{

    char* functionName = getFunctionName(b1);
    int functionSkip = 0;
    functionSkip  = (strncmp(functionName, "prepareStackTrace", 17) == 0);
    functionSkip |= (strncmp(functionName, "promiseRejectHandler", 20) == 0);
    if (functionSkip) {
        return detourFunctionCallOrig(b1, b2, a1, a2, a3, argc, argv);
    }


    int skipped = 0;

    for (int i = 0; i < argc; i++) {
        auto value = argv[i];
        uint8_t* p = *(uint8_t**)*value;


        // NOTE: Define function here to bypass it being catched.
        Function_Stringify = Function_StringifyPtr(PPP);
        
        if (p != NULL && !IsSMI(p)) {
            int ttype = getTTtype((uint8_t*)*value);
            char* name = 0;
            if (ttype == ONE_BYTE_STRING_TYPE) {
                name = readFlatString(p).data;
            }
            if (name != NULL) {
                int skip = 0;
                //skip  = (strncmp(name, "-ipc-", 4) == 0);
                //skip |= (strncmp(name, "console-message", 15) == 0);

                if (skip) {
                    skipped = 1;
                    break;
                }
            }
        }

        if (p != NULL) {

            if (IsSMI(p)) {
                //logfff("%d: %x,  SMI: %d\n", i, p, ExtractSMIValue(p));
                logfff(1, "%d: [SMI] %d\n", i, ExtractSMIValue(p));
            }
            else if ((*p) == 0) {
                // TODO: Why SMI didn't catch it?   
                //logfff("%d: 0 ", i);
                skipped = true;
                break;
            }
            else {
                //v8::MaybeLocal<v8::String> result = {};
                //v8::Local<v8::String> string = {};
                //v8::MaybeLocal<v8::String>* x = Function_Stringify(&result, a2, value, string);

                //int ttype = getTTtype(*(uint8_t**)x);
                //if (ttype < 0 && i == 0) {
                //    // TODO: Look into why its does not have a value
                //    //skipped = true;
                //    //break;
                //}

                //if (ttype == ONE_BYTE_STRING_TYPE) {
                //    logfff("%d: [%x %d] ", i, ttype, ttype);
                //    loc_buff data = testReadFlatString((uint8_t*)x);
                //    testPrintWithoutNewLines(data.data, data.size);
                //    logfff("\n");
                //}
                //else if (ttype == CONS_ONE_BYTE_STRING_TYPE) {
                //    logfff("%d: [%x %d] ", i, ttype, ttype);
                //    loc_buff data = testReadConsString(*(uint8_t**)x);
                //    logfff("\n");
                //}
                //else if (ttype == CONS_STRING_TYPE) {
                //    logfff("%d: [%x %d] ", i, ttype, ttype);
                //    loc_buff data = testReadConsString(*(uint8_t**)x);
                //    logfff("\n");
                //}
                //else if (ttype == ODDBALL_TYPE) {
                //    logfff("%d: [%x %d] ", i, ttype, ttype);
                //    logfff("%s\n", getOddBallType(*value));
                //}
                //else {
                //    if (ttype == -1) { ttype = 0;  }
                //    logfff("%d: [%x %d] ", i, ttype, ttype);
                //    logfff("_______\n");
                //}


                // =====================================================================
                // =====================================================================
                // =====================================================================

                int ttype = getTTtype((uint8_t*)*value);
                logfff(1, "%d: %x TYPE: %x %d, ", i, p, ttype, ttype);

                if (ttype == ONE_BYTE_STRING_TYPE) {
                    loc_buff buff = readFlatString(p);
                    logfff(1, "%.*s\n", buff.size, buff.data);
                }
                else if (ttype == CONS_ONE_BYTE_STRING_TYPE) {
                    logfff(1, "%d: [%x %d] ", i, ttype, ttype);
                    //loc_buff data = testReadConsString(*(uint8_t**)x);
                    loc_buff data = testReadConsString(p);
                    logfff(1, "\n");
                }
                else if (ttype == JS_OBJECT_TYPE) {
                    logfff(1, "JS_OBJECT_TYPE: %x\n", p);
                }
                else if (ttype == ODDBALL_TYPE) {
                    logfff(1, "-- %s\n", getOddBallType(*value));
                }
                else {
                    logfff(1, "\n");
                }
            }
        }
        else {
            if (i == 0) {
              /*  skipped = true;
                break;*/
            }
            logfff(1, "%d: NULL VALUE\n", i);
        }
    }

    if (!skipped) {
        logfff(1, "\n");
    }

    return detourFunctionCallOrig(b1, b2, a1, a2, a3, argc, argv);
}


void* (__cdecl* detourEmitEvent1Orig)(void* a1, void* a2, void* a3, char** a4, void* a5, void* a6, void* a7, void* a8);
void* __cdecl  detourEmitEvent1(void* a1, void* a2, void* a3, char** a4, void* a5, void* a6, void* a7, void* a8)
{
    logfff(1, "[1] %s \n", *a4);

    //ipc-message-sync, ipc-message
    return detourEmitEvent1Orig(a1, a2, a3, a4, a5, a6, a7, a8);
}

void* (__cdecl* detourEmitEvent2Orig)(void* a1, void* a2, void* a3, char** a4, void* a5);
void* __cdecl  detourEmitEvent2(void* a1, void* a2, void* a3, char** a4, void* a5)
{
    logfff(1, "[2] %s \n", *a4);

    // blur, focus, restore, resize
    return detourEmitEvent2Orig(a1, a2, a3, a4, a5);
}

void* (__cdecl* detourEmitEvent3Orig)(void* a1, void* a2, void* a3, char** a4, void* a5);
void* __cdecl  detourEmitEvent3(void* a1, void* a2, void* a3, char** a4, void* a5)
{
    logfff(1, "[2] %s \n", *a4);

    // blur, focus, restore, resize
    return detourEmitEvent3Orig(a1, a2, a3, a4, a5);
}

void* (__cdecl* detourEmitEvent4Orig)(void* a1, void* a2, void* a3, char** a4, void* a5, void* a6);
void* __cdecl  detourEmitEvent4(void* a1, void* a2, void* a3, char** a4, void* a5, void* a6)
{
    logfff(1, "[2] %s \n", *a4);

    // blur, focus, restore, resize
    return detourEmitEvent4Orig(a1, a2, a3, a4, a5, a6);
}

void* (__cdecl* detourEmitEvent5Orig)(void* a1, void* a2, void* a3, char** a4, void* a5, void* a6);
void* __cdecl  detourEmitEvent5(void* a1, void* a2, void* a3, char** a4, void* a5, void* a6)
{
    logfff(1, "[2] %s \n", *a4);

    // blur, focus, restore, resize
    return detourEmitEvent5Orig(a1, a2, a3, a4, a5, a6);
}

void* (__cdecl* detourEmitEvent6Orig)(void* a1, void* a2, void* a3, char** a4, void* a5, void* a6);
void* __cdecl  detourEmitEvent6(void* a1, void* a2, void* a3, char** a4, void* a5, void* a6)
{
    logfff(1, "[2] %s \n", *a4);

    // blur, focus, restore, resize
    return detourEmitEvent5Orig(a1, a2, a3, a4, a5, a6);
}





//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------




void createEventMonitoringHook() {
    const char* signature = "8B 45 14 89 44 24 0C 8B 45 10 89 44 24 08 0F 57 C0 0F 11 44 24 18 89 7C 24 04 89 34 24 E8 ? ? ? ? 83 C4 28 8B 06 85 C0";
    int offsetToTop = -0x48;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    logfff(1, "Looked for: %s, found Location: %x\n", signature, location);

    if (location != 0) {
        void* HookFunctionName = (void*)(location);
        MH_CreateHook(HookFunctionName, &detourCallMethodWithArgs, reinterpret_cast<void**>(&detourCallMethodWithArgsOrig));
    }
}


void createFunctionMonitoringHook() {
    const char* signature = "BB 68 AE FF FF 03 58 08 A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? C7 84 24 90 00 00 00 00 00 00 00 8A 08 F6 C1 05 0F 85 ? ? ? ? 8B 83 64 1B 00 00 3B 43 54 0F 85 ? ? ? ? 8B 7B 54 8B 83 40 5B 00 00 3B 83 44 5B 00 00 0F 84 ? ? ? ? 8D 48 04 89 8B 40 5B 00 00 89 44 24 08 89 38";
    int offsetToTop = -0x2b;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    logfff(1, "Looked for: %s, found Location: %x\n", signature, location);

    if (location != 0) {
        void* HookFunctionName = (void*)(location);
        MH_CreateHook(HookFunctionName, &detourFunctionCall, reinterpret_cast<void**>(&detourFunctionCallOrig));
    }
}

void createEmitEvent2Hook() {
    const char* signature = "A1 ? ? ? ? 31 E8 89 45 F0 31 C0 48 8D 5D E4 89 03 89 43 04 89 43 08 8B 75 0C 8B 45 14 8D 7D DC 50 56 57 E8 ? ? ? ? 83 C4 0C 8B 07 89 07 8D 7D E0 8B 45 18 FF 30 56 57 E8 ? ? ? ? 83 C4 0C 6A 08 E8 ? ? ? ? 83 C4 04 89 03 8D 48 08 89 4B 08 F2 0F 10 47 FC F2 0F 11 00 89 4B 04";
    int offsetToTop = -0x9;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    logfff(1, "Looked for: %s, found Location: %x\n", signature, location);

    if (location != 0) {
        void* HookFunctionName = (void*)(location);
        MH_CreateHook(HookFunctionName, &detourEmitEvent2, reinterpret_cast<void**>(&detourEmitEvent2Orig));
    }
}

void createEmitEvent3Hook() {
    const char* signature = "31 E8 89 45 F0 31 C0 48 8D 5D D8 89 03 89 43 04 89 43 08 8B 7D 0C 8B 45 14 8D 75 E4 50 57 56 E8 ? ? ? ? 83 C4 0C 8B 06 89 06 8D 45 E8 8B 4D 18 FF 31 57 50 E8 ? ? ? ? 83 C4 0C 8D 75 EC 8B 45 1C FF 30 57 56 E8 ? ? ? ? 83 C4 0C 6A 0C E8 ? ? ? ? 83 C4 04 89 03 8D 48 0C 89 4B 08 F2 0F 10 46 F8 F2 0F 11 00 8B 16 8B 75 08 89 50 08 89 4B 04 53 68 ? ? ? ? FF 75 10 57 56 E8 ? ? ? ? 83 C4 14 8B 03 85 C0 74 0C";
    int offsetToTop = -0xE;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    logfff(1, "Looked for: %s, found Location: %x\n", signature, location);

    if (location != 0) {
        void* HookFunctionName = (void*)(location);
        MH_CreateHook(HookFunctionName, &detourEmitEvent3, reinterpret_cast<void**>(&detourEmitEvent3Orig));
    }
}

void createEmitEvent4Hook() {
    const char* signature = "8D 5D D8 89 03 89 43 04 89 43 08 8B 7D 0C 8B 45 14 8D 75 E4 50 57 56 E8 ? ? ? ? 83 C4 0C 8B 06 89 06 8D 45 E8 8B 4D 18 FF 31 57 50 E8 ? ? ? ? 83 C4 0C 8D 75 EC FF 75 1C 57 56 E8 ? ? ? ? 83 C4 0C 6A 0C E8 ? ? ? ? 83 C4 04 89 03 8D 48 0C 89 4B 08 F2 0F 10 46 F8 F2 0F 11 00 8B 16 8B 75 08 89 50 08 89 4B 04 53 68 ? ? ? ? FF 75 10 57 56 E8 ? ? ? ? 83 C4 14 8B 03 85 C0 74 0C 89 45 DC 50 E8 ? ? ? ? 83 C4 04 8B 4D F0 31 E9";
    int offsetToTop = -0x16;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    logfff(1, "Looked for: %s, found Location: %x\n", signature, location);

    if (location != 0) {
        void* HookFunctionName = (void*)(location);
        MH_CreateHook(HookFunctionName, &detourEmitEvent4, reinterpret_cast<void**>(&detourEmitEvent4Orig));
    }
}



void createEmitEvent5Hook() {
    // TODO: Hook hooks into even4. Needs deeper look. Do it later, maybe. Not important.

    //const char* signature = "A1 ? ? ? ? 31 E8 89 45 F0 31 C0 48 8D 5D D8 89 03 89 43 04 89 43 08 8B 7D 0C 8B 45 14 8D 75 E4 50 57 56 E8 ? ? ? ? 83 C4 0C 8B 06 89 06 8D 45 E8 8B 4D 18 FF 31 57 50 E8 ? ? ? ? 83 C4 0C 8D 75 EC FF 75 1C 57 56 E8 ? ? ? ? 83 C4 0C 6A 0C E8 ? ? ? ? 83 C4 04 89 03 8D 48 0C 89 4B 08 F2 0F 10 46 F8 F2 0F 11 00 8B 16 8B 75 08 89 50 08 89 4B 04 53 68 ? ? ? ? FF 75 10 57 56 E8 ? ? ? ? 83 C4 14 8B 03 85 C0 74 0C 89 45 DC 50 E8 ? ? ? ? 83 C4 04 8B 4D F0 31 E9 E8";
    //int offsetToTop = -0x9;

    //unsigned char* location = findSignature(signature);
    //location = location + offsetToTop;

    //logfff(1, "Looked for: %s, found Location: %x\n", signature, location);

    //if (location != 0) {
    //    void* HookFunctionName = (void*)(location);
    //    MH_CreateHook(HookFunctionName, &detourEmitEvent5, reinterpret_cast<void**>(&detourEmitEvent5Orig));
    //}
}

void createEmitEvent6Hook() {
    // TODO: Hook hooks into even4. Needs deeper look. Do it later, maybe. Not important.
    //const char* signature = "31 E8 89 45 F0 31 C0 48 8D 5D D8 89 03 89 43 04 89 43 08 8B 7D 0C 8B 45 14 8D 75 E4 50 57 56 E8 ? ? ? ? 83 C4 0C 8B 06 89 06 8D 45 E8 8B 4D 18 FF 31 57 50 E8 ? ? ? ? 83 C4 0C 8D 75 EC FF 75 1C 57 56 E8 ? ? ? ? 83 C4 0C 6A 0C E8 ? ? ? ? 83 C4 04 89 03 8D 48 0C 89 4B 08 F2 0F 10 46 F8 F2 0F 11 00 8B 16 8B 75 08 89 50 08 89 4B 04 53 68 ? ? ? ? FF 75 10 57 56 E8 ? ? ? ? 83 C4 14 8B 03 85 C0 74 0C 89 45 DC 50 E8 ? ? ? ? 83 C4 04 8B 4D F0 31 E9 E8";
    //int offsetToTop = -0xE;

    //unsigned char* location = findSignature(signature);
    //location = location + offsetToTop;

    //logfff(1, "Looked for: %s, found Location: %x\n", signature, location);

    //if (location != 0) {
    //    void* HookFunctionName = (void*)(location);
    //    MH_CreateHook(HookFunctionName, &detourEmitEvent6, reinterpret_cast<void**>(&detourEmitEvent6Orig));
    //}
}


void createCallMethodWithArgsHook() {
    const char* signature = "8B 7D 0C A1 ? ? ? ? 31 E8 89 45 F0 31 C0 48 8D 4D EC 89 01 8B 5D 18 6A 00 6A 01 57 E8 ? ? ? ? 8D 75 E8 31 C0 48 89 06 8B 03 8B 4B 04 29 C1 C1 F9 02 83 EC 28 89 44 24 14 89 4C 24 10 8B 45 14 89 44 24 0C 8B 45 10 89 44 24 08 0F 57 C0 0F 11 44 24 18 89 7C 24 04 89 34 24 E8 ? ? ? ? 83 C4 28 8B 06 85 C0 74 07 8B 75 08 89 06 EB 08 83 C7 60 8B 75 08 89 3E 8D 4D EC E8 ? ? ? ? 8B 4D F0 31 E9 E8 ? ? ? ? 89 F0";
    int offsetToTop = -0x9;

    unsigned char* location = findSignature(signature);
    location = location + offsetToTop;

    logfff(1, "Looked for: %s, found Location: %x\n", signature, location);

    if (location != 0) {
        void* HookFunctionName = (void*)(location);
        MH_CreateHook(HookFunctionName, &detourCallMethodWithArgs, reinterpret_cast<void**>(&detourCallMethodWithArgsOrig));
    }
}



void createTestHooks() {
    // TODO: Refactor into common format.

    logfff(1, "-------------- create test hook");

    const char* signature = "55 89 E5 53 57 56 83 EC 0C 8B 75 08 A1 ? ? ? ? 31 E8 89 45 F0 8B 01 89 C2 81 E2 00 00 FC FF BF 68 AE FF FF 03 7A 08 8B 50 FF 0F B7 52 07 0F B7 D2 81 FA 0B 08 00 00 75 2C 8D 5D EC 51 57 53 E8 ? ? ? ? 83 C4 0C 8B 03 85 C0 74 57 89 06 8B 4D F0 31 E9 E8 ? ? ? ? 89 F0 83 C4 0C 5E 5F 5B 5D C2 04 00";
    int offsetToTop = -0x0;
    unsigned char* location = findSignature(signature);
    //Function_GetName = v8::Function::GetNamePtr(location);
    Function_GetName = GetNamePtr(location);



    signature = "5D C2 08 00 8B 4D 0C 85 C9 0F 84 ? ? ? ? B8 00 00 FC FF 23 01 BB 68 AE FF FF 03 58 08 8B 83 64 1B 00 00 3B 43 54 0F 85 ? ? ? ? 8B 53 54 8B 83 40 5B 00 00 3B 83 44 5B 00 00 0F 84 ? ? ? ? 8D 48 04 89 8B 40 5B 00 00 89 44 24 04 89 10 E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 8B 83 40 5B 00 00 89 44 24 18 8B 83 44 5B 00 00 89 44 24 08 FF 83 48 5B 00 00 8D 4C 24 40 FF 75 0C 53 E8 ? ? ? ? 31 C0 8D 4C 24 28 89 41 F8 0F 57 C0 0F 11 01";
    offsetToTop = -0x44;
    location = findSignature(signature);
    location = location + offsetToTop;
    Function_ValueToString = ValueToStringPtr(location);


    signature = "89 4D D0 8B 7D 10 A1 ? ? ? ? 31 E8 89 45 F0 89 F9 E8 ? ? ? ? 31 C0 48 8D 75 E4 89 06 89 46 04 89 46 08 89 F9 E8 ? ? ? ? 89 F1 50 E8 ? ? ? ? 8D 45 D8 31 C9 89 08 89 48 04 89 40 FC C7 40 08 64 00 00 00 89 F9 E8 ? ? ? ? 8D 5D D4 8B 4D D0 50 FF 75 0C 53 FF 75 08 E8 ? ? ? ? 89 D9 FF 73 04 E8 ? ? ? ? 89 F1 E8 ? ? ? ? 89 F9 E8 ? ? ? ? 8B 4D F0 31 E9 E8";
    offsetToTop = -0x9;
    location = findSignature(signature);
    location = location + offsetToTop;
    FromV8ValuePtr Function_FromV8Value = FromV8ValuePtr(location);


    signature = "8B 5D 08 8B 75 0C A1 ? ? ? ? 31 E8 89 44 24 68 85 F6 0F 84 ? ? ? ? B8 00 00 FC FF 23 06 BF 68 AE FF FF 03 78 08 8B 87 64 1B 00 00 3B 47 54 74 13 3B 87 C4 00 00 00 75 0B C7 03 00 00 00 00 E9 ? ? ? ? 8D 4C 24 58 57 E8 ? ? ? ? 8D 4C 24 30 56 57 E8 ? ? ? ? 31 C0 8D 54 24 18 89 42 F8 0F 57 C0 0F 11 02 89 42 14 89 42 10 A1 ? ? ? ? 85 C0 0F 85";
    offsetToTop = -0xC;
    location = findSignature(signature);
    location = location + offsetToTop;
    Function_StringifyPtr Function_Stringify = (Function_StringifyPtr)location;
    PPP = location;




    logfff(1, "Signature f: %x\n", location);
  
    createFunctionMonitoringHook();
    
    //createEventMonitoringHook();
    //createEmitEvent1Hook();
    //createEmitEvent2Hook();
    //createEmitEvent3Hook();
    //createEmitEvent4Hook();
    //createEmitEvent5Hook();
    //createEmitEvent6Hook();
    //createCallMethodWithArgsHook();
}
