#pragma once

#include <limits>
#include <iostream>
#include <span>
#include "external_data/object.h"

// #define UNSAFE_TODO(...) UNSAFE_BUFFERS(__VA_ARGS__)


struct WebSocketFrameHeader {
	typedef int OpCode;

	enum OpCodeEnum {
		kOpCodeContinuation = 0x0,
		kOpCodeText = 0x1,
		kOpCodeBinary = 0x2,
		kOpCodeDataUnused3 = 0x3,
		kOpCodeDataUnused4 = 0x4,
		kOpCodeDataUnused5 = 0x5,
		kOpCodeDataUnused6 = 0x6,
		kOpCodeDataUnused7 = 0x7,
		kOpCodeClose = 0x8,
		kOpCodePing = 0x9,
		kOpCodePong = 0xA,
		kOpCodeControlUnusedB = 0xB,
		kOpCodeControlUnusedC = 0xC,
		kOpCodeControlUnusedD = 0xD,
		kOpCodeControlUnusedE = 0xE,
		kOpCodeControlUnusedF = 0xF,
	};

	// Return true if |opcode| is one of the data opcodes known to this
	// implementation.
	static bool IsKnownDataOpCode(OpCode opcode) {
		return opcode == kOpCodeContinuation || opcode == kOpCodeText ||
			opcode == kOpCodeBinary;
	}

	// Return true if |opcode| is one of the control opcodes known to this
	// implementation.
	static bool IsKnownControlOpCode(OpCode opcode) {
		return opcode == kOpCodeClose || opcode == kOpCodePing ||
			opcode == kOpCodePong;
	}

	// Return true if |opcode| is one of the reserved data opcodes.
	static bool IsReservedDataOpCode(OpCode opcode) {
		return opcode == kOpCodeDataUnused3 || opcode == kOpCodeDataUnused4 ||
			opcode == kOpCodeDataUnused5 || opcode == kOpCodeDataUnused6 ||
			opcode == kOpCodeDataUnused7;
	}

	// Return true if |opcode| is one of the reserved control opcodes.
	static bool IsReservedControlOpCode(OpCode opcode) {
		return opcode == kOpCodeControlUnusedB || opcode == kOpCodeControlUnusedC ||
			opcode == kOpCodeControlUnusedD || opcode == kOpCodeControlUnusedE ||
			opcode == kOpCodeControlUnusedF;
	}

	// These values must be compile-time constants.
	static constexpr size_t kBaseHeaderSize = 2;
	static constexpr size_t kMaximumExtendedLengthSize = 8;
	static constexpr size_t kMaskingKeyLength = 4;

	// Contains four-byte data representing "masking key" of WebSocket frames.
	struct WebSocketMaskingKey {
		std::array<uint8_t, WebSocketFrameHeader::kMaskingKeyLength> key;
	};

	// Constructor to avoid a lot of repetitive initialisation.
	explicit WebSocketFrameHeader(OpCode opCode) : opcode(opCode) {}

	WebSocketFrameHeader(const WebSocketFrameHeader&) = delete;
	WebSocketFrameHeader& operator=(const WebSocketFrameHeader&) = delete;

	// Create a clone of this object on the heap.
	std::unique_ptr<WebSocketFrameHeader> Clone() const;

	// Overwrite this object with the fields from |source|.
	void CopyFrom(const WebSocketFrameHeader& source);

	// Members below correspond to each item in WebSocket frame header.
	// See <http://tools.ietf.org/html/rfc6455#section-5.2> for details.
	bool final = false;
	bool reserved1 = false;
	bool reserved2 = false;
	bool reserved3 = false;
	OpCode opcode;
	bool masked = false;
	WebSocketMaskingKey masking_key = {};
	uint64_t payload_length = 0;
};

//constexpr inline auto DanglingUntriaged = base::RawPtrTraits::kMayDangle;
constexpr auto DanglingUntriaged = 1;

struct WebSocketFrame {
	explicit WebSocketFrame(WebSocketFrameHeader::OpCode opcode);

	WebSocketFrame(const WebSocketFrame&) = delete;
	WebSocketFrame& operator=(const WebSocketFrame&) = delete;

	~WebSocketFrame() {}; // Proper desctructor?

	WebSocketFrameHeader header;

	//base::raw_span<const uint8_t, DanglingUntriaged> payload;
	std::span<const uint8_t, DanglingUntriaged> payload;

};




// -----------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------






// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------


// // Default traits for raw pointers
// struct DefaultRawPtrTraits {
//     // Called when the pointer is reset
//     template <typename T>
//     static void OnReset(T* ptr) {
//         // No-op in the default case
//     }
//
//     // Called when the pointer is destroyed
//     template <typename T>
//     static void OnDestroy(T* ptr) {
//         // No-op in the default case
//     }
//
//     // Optional: Tagging and untagging for custom behavior
//     template <typename T>
//     static T* Tag(T* ptr) {
//         return ptr; // No tagging in default case
//     }
//
//     template <typename T>
//     static T* Untag(T* ptr) {
//         return ptr; // No untagging in default case
//     }
// };
//
// // The raw_ptr class with customizable traits
// template <typename T, typename Traits = DefaultRawPtrTraits>
// class raw_ptr {
// private:
//     T* ptr;
//
// public:
//     // Constructor
//     raw_ptr() : ptr(nullptr) {}
//     explicit raw_ptr(T* p) : ptr(Traits::Tag(p)) {}
//
//     // Destructor
//     ~raw_ptr() {
//         Traits::OnDestroy(Traits::Untag(ptr));
//         ptr = nullptr;
//     }
//
//     // Access operators
//     T& operator*() const {
//         assert(ptr != nullptr);
//         return *Traits::Untag(ptr);
//     }
//
//     T* operator->() const {
//         assert(ptr != nullptr);
//         return Traits::Untag(ptr);
//     }
//
//     // Reset pointer
//     void reset(T* p = nullptr) {
//         Traits::OnReset(Traits::Untag(ptr));
//         ptr = Traits::Tag(p);
//     }
//
//     // Get raw pointer
//     T* get() const {
//         return Traits::Untag(ptr);
//     }
//
//     // Null check
//     bool is_null() const { return ptr == nullptr; }
// };
//
// // Example custom traits for debugging
// struct DebugRawPtrTraits {
//     template <typename T>
//     static void OnReset(T* ptr) {
//         if (ptr) {
//             std::cout << "Resetting pointer: " << ptr << std::endl;
//         }
//     }
//
//     template <typename T>
//     static void OnDestroy(T* ptr) {
//         if (ptr) {
//             std::cout << "Destroying pointer: " << ptr << std::endl;
//         }
//     }
//
//     template <typename T>
//     static T* Tag(T* ptr) {
//         // Example: Add a simple offset as a "tag" (not real-world safe)
//         return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) + 1);
//     }
//
//     template <typename T>
//     static T* Untag(T* ptr) {
//         return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) - 1);
//     }
// };
//
//
// // template <typename T>
// // struct RawPtrTraits {
// //     using element_type = T;
// //
// //     // Check if the pointer is null
// //     static bool is_null(T* ptr) {
// //         return ptr == nullptr;
// //     }
// //
// //     // Reset the pointer
// //     static void reset(T*& ptr) {
// //         delete ptr;
// //         ptr = nullptr;
// //     }
// //
// //     // Access the raw pointer
// //     static T* get(T* ptr) {
// //         return ptr;
// //     }
// // };
//
//
// // template <typename T, typename Traits = void>
// // class raw_ptr {
// // private:
// //     T* ptr;
// //
// // public:
// //     // Constructors
// //     raw_ptr() : ptr(nullptr) {}
// //     explicit raw_ptr(T* p) : ptr(p) {}
// //
// //     // Destructor
// //     ~raw_ptr() { ptr = nullptr; } // Prevent dangling
// //
// //     // Copy and move
// //     raw_ptr(const raw_ptr&) = default;
// //     raw_ptr& operator=(const raw_ptr&) = default;
// //
// //     raw_ptr(raw_ptr&& other) noexcept : ptr(other.ptr) {
// //         other.ptr = nullptr;
// //     }
// //
// //     raw_ptr& operator=(raw_ptr&& other) noexcept {
// //         if (this != &other) {
// //             ptr = other.ptr;
// //             other.ptr = nullptr;
// //         }
// //         return *this;
// //     }
// //
// //     // Access operators
// //     T& operator*() const { return *ptr; }
// //     T* operator->() const { return ptr; }
// //
// //     // Getter
// //     T* get() const { return ptr; }
// //
// //     // Reset
// //     void reset(T* p = nullptr) { ptr = p; }
// //
// //     // Null check
// //     bool is_null() const { return ptr == nullptr; }
// // };
//
//
//
// template <class T> class scoped_refptr;
//
//     template <class, typename> class RefCounted;
//     template <class, typename> class RefCountedThreadSafe;
//     template <class> class RefCountedDeleteOnSequence;
//     class SequencedTaskRunner;
//     template <typename T> scoped_refptr<T> AdoptRef(T* t);
//         enum AdoptRefTag { kAdoptRefTag };
//         enum StartRefCountFromZeroTag { kStartRefCountFromZeroTag };
//         enum StartRefCountFromOneTag { kStartRefCountFromOneTag };
//         template <typename TagType>
//             struct RefCountPreferenceTagTraits;
//         template <>
//             struct RefCountPreferenceTagTraits<StartRefCountFromZeroTag> {
//                 static constexpr StartRefCountFromZeroTag kTag = kStartRefCountFromZeroTag;
//             };
//         template <>
//             struct RefCountPreferenceTagTraits<StartRefCountFromOneTag> {
//                 static constexpr StartRefCountFromOneTag kTag = kStartRefCountFromOneTag;
//             };
//         template <typename T, typename Tag = typename T::RefCountPreferenceTag>
//             constexpr Tag GetRefCountPreference() {
//                 return RefCountPreferenceTagTraits<Tag>::kTag;
//             }
//         // scoped_refptr<T> is typically used with one of several RefCounted<T> base
//         // classes or with custom AddRef and Release methods. These overloads dispatch
//         // on which was used.
//         template <typename T, typename U, typename V>
//             constexpr bool IsRefCountPreferenceOverridden(const T*,
//                     const RefCounted<U, V>*) {
//                 return !std::same_as<std::decay_t<decltype(GetRefCountPreference<T>())>,
//                 std::decay_t<decltype(GetRefCountPreference<U>())>>;
//             }
//         template <typename T, typename U, typename V>
//             constexpr bool IsRefCountPreferenceOverridden(
//                     const T*,
//                     const RefCountedThreadSafe<U, V>*) {
//                 return !std::same_as<std::decay_t<decltype(GetRefCountPreference<T>())>,
//                 std::decay_t<decltype(GetRefCountPreference<U>())>>;
//             }
//         template <typename T, typename U>
//             constexpr bool IsRefCountPreferenceOverridden(
//                     const T*,
//                     const RefCountedDeleteOnSequence<U>*) {
//                 return !std::same_as<std::decay_t<decltype(GetRefCountPreference<T>())>,
//                 std::decay_t<decltype(GetRefCountPreference<U>())>>;
//             }
//         constexpr bool IsRefCountPreferenceOverridden(...) {
//             return false;
//         }
//         template <typename T, typename U, typename V>
//             constexpr void AssertRefCountBaseMatches(const T*, const RefCounted<U, V>*) {
//                 static_assert(std::derived_from<T, U>,
//                         "T implements RefCounted<U>, but U is not a base of T.");
//             }
//         template <typename T, typename U, typename V>
//             constexpr void AssertRefCountBaseMatches(const T*,
//                     const RefCountedThreadSafe<U, V>*) {
//                 static_assert(
//                         std::derived_from<T, U>,
//                         "T implements RefCountedThreadSafe<U>, but U is not a base of T.");
//             }
//         template <typename T, typename U>
//             constexpr void AssertRefCountBaseMatches(const T*,
//                     const RefCountedDeleteOnSequence<U>*) {
//                 static_assert(
//                         std::derived_from<T, U>,
//                         "T implements RefCountedDeleteOnSequence<U>, but U is not a base of T.");
//             }
//         constexpr void AssertRefCountBaseMatches(...) {}
//
//     template <typename T>
//         scoped_refptr<T> AdoptRef(T* obj) {
//             using Tag = std::decay_t<decltype(GetRefCountPreference<T>())>;
//             static_assert(std::same_as<StartRefCountFromOneTag, Tag>,
//                     "Use AdoptRef only if the reference count starts from one.");
//             DCHECK(obj);
//             DCHECK(obj->HasOneRef());
//             obj->Adopted();
//             return scoped_refptr<T>(obj, kAdoptRefTag);
//         }
//         template <typename T>
//             scoped_refptr<T> AdoptRefIfNeeded(T* obj, StartRefCountFromZeroTag) {
//                 return scoped_refptr<T>(obj);
//             }
//         template <typename T>
//             scoped_refptr<T> AdoptRefIfNeeded(T* obj, StartRefCountFromOneTag) {
//                 return AdoptRef(obj);
//             }
//     template <typename T, typename... Args>
//         scoped_refptr<T> MakeRefCounted(Args&&... args) {
//             T* obj = new T(std::forward<Args>(args)...);
//             return AdoptRefIfNeeded(obj, GetRefCountPreference<T>());
//         }
//     // Takes an instance of T, which is a ref counted type, and wraps the object
//     // into a scoped_refptr<T>.
//     template <typename T>
//         scoped_refptr<T> WrapRefCounted(T* t) {
//             return scoped_refptr<T>(t);
//         }
//     //template <typename T, RawPtrTraits Traits = RawPtrTraits::kEmpty>
//     //    scoped_refptr<T> WrapRefCounted(const raw_ptr<T, Traits>& t) {
//     //        return scoped_refptr<T>(t.get());
//     //    }
//
//
// template <class T>
// class  scoped_refptr {
//     public:
//         typedef T element_type;
//         constexpr scoped_refptr() = default;
//         // Allow implicit construction from nullptr.
//         constexpr scoped_refptr(std::nullptr_t) {}
//         // Constructs from a raw pointer. Note that this constructor allows implicit
//         // conversion from T* to scoped_refptr<T> which is strongly discouraged. If
//         // you are creating a new ref-counted object please use
//         // base::MakeRefCounted<T>() or base::WrapRefCounted<T>(). Otherwise you
//         // should move or copy construct from an existing scoped_refptr<T> to the
//         // ref-counted object.
//         scoped_refptr(T* p) : ptr_(p) {
//             if (ptr_) {
//                 AddRef(ptr_);
//             }
//         }
//         // Copy constructor. This is required in addition to the copy conversion
//         // constructor below.
//         scoped_refptr(const scoped_refptr& r) : scoped_refptr(r.ptr_) {}
//         // Copy conversion constructor.
//         template <typename U>
//             requires(std::convertible_to<U*, T*>)
//             scoped_refptr(const scoped_refptr<U>& r) : scoped_refptr(r.ptr_) {}
//         // Move constructor. This is required in addition to the move conversion
//         // constructor below.
//         scoped_refptr(scoped_refptr&& r) noexcept : ptr_(r.ptr_) { r.ptr_ = nullptr; }
//         // Move conversion constructor.
//         template <typename U>
//             requires(std::convertible_to<U*, T*>)
//             scoped_refptr(scoped_refptr<U>&& r) noexcept : ptr_(r.ptr_) {
//                 r.ptr_ = nullptr;
//             }
//         ~scoped_refptr() {
//             // static_assert(IsRefCountPreferenceOverridden(
//             //             static_cast<T*>(nullptr), static_cast<T*>(nullptr)),
//             //         "It's unsafe to override the ref count preference."
//             //         " Please remove REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE"
//             //         " from subclasses.");
//             if (ptr_) {
//                 Release(ptr_);
//             }
//         }
//         T* get() const { return ptr_; }
//         T& operator*() const {
//             DCHECK(ptr_);
//             return *ptr_;
//         }
//         T* operator->() const {
//             return ptr_;
//         }
//         scoped_refptr& operator=(std::nullptr_t) {
//             reset();
//             return *this;
//         }
//         scoped_refptr& operator=(T* p) { return *this = scoped_refptr(p); }
//         // Unified assignment operator.
//         scoped_refptr& operator=(scoped_refptr r) noexcept {
//             swap(r);
//             return *this;
//         }
//         // Sets managed object to null and releases reference to the previous managed
//         // object, if it existed.
//         void reset() { scoped_refptr().swap(*this); }
//         // Returns the owned pointer (if any), releasing ownership to the caller. The
//         // caller is responsible for managing the lifetime of the reference.
//         [[nodiscard]] T* release();
//         void swap(scoped_refptr& r) noexcept { std::swap(ptr_, r.ptr_); }
//         explicit operator bool() const { return ptr_ != nullptr; }
//         template <typename U>
//             friend bool operator==(const scoped_refptr<T>& lhs,
//                     const scoped_refptr<U>& rhs) {
//                 return lhs.ptr_ == rhs.ptr_;
//             }
//         // This operator is an optimization to avoid implicitly constructing a
//         // scoped_refptr<U> when comparing scoped_refptr against raw pointer. If the
//         // implicit conversion is ever removed this operator can also be removed.
//         template <typename U>
//             friend bool operator==(const scoped_refptr<T>& lhs, const U* rhs) {
//                 return lhs.ptr_ == rhs;
//             }
//         friend bool operator==(const scoped_refptr<T>& lhs, std::nullptr_t null) {
//             return !static_cast<bool>(lhs);
//         }
//         template <typename U>
//             friend auto operator<=>(const scoped_refptr<T>& lhs,
//                     const scoped_refptr<U>& rhs) {
//                 return lhs.ptr_ <=> rhs.ptr_;
//             }
//         friend auto operator<=>(const scoped_refptr<T>& lhs, std::nullptr_t null) {
//             return lhs.ptr_ <=> static_cast<T*>(nullptr);
//         }
//     protected:
//         // RAW_PTR_EXCLUSION: scoped_refptr<> has its own UaF prevention mechanism.
//         // Given how widespread it is, we it'll likely a perf regression for no
//         // additional security benefit.
//         T* ptr_ = nullptr;
//     private:
//         template <typename U>
//             friend scoped_refptr<U> AdoptRef(U*);
//         friend class SequencedTaskRunner;
//         scoped_refptr(T* p, AdoptRefTag) : ptr_(p) {}
//         // Friend required for move constructors that set r.ptr_ to null.
//         template <typename U>
//             friend class scoped_refptr;
//         // Non-inline helpers to allow:
//         //     class Opaque;
//         //     extern template class scoped_refptr<Opaque>;
//         // Otherwise the compiler will complain that Opaque is an incomplete type.
//         static void AddRef(T* ptr);
//         static void Release(T* ptr);
// };
// template <typename T>
// T* scoped_refptr<T>::release() {
//     T* ptr = ptr_;
//     ptr_ = nullptr;
//     return ptr;
// }
// // static
// template <typename T>
// void scoped_refptr<T>::AddRef(T* ptr) {
//     AssertRefCountBaseMatches(ptr, ptr);
//     ptr->AddRef();
// }
// // static
// template <typename T>
// void scoped_refptr<T>::Release(T* ptr) {
//     AssertRefCountBaseMatches(ptr, ptr);
//     ptr->Release();
// }
// template <typename T>
// std::ostream& operator<<(std::ostream& out, const scoped_refptr<T>& p) {
//     return out << p.get();
// }
// template <typename T>
// void swap(scoped_refptr<T>& lhs, scoped_refptr<T>& rhs) noexcept {
//     lhs.swap(rhs);
// }
//
// // ----------------------------------------------------------------------------------------------------
// // ----------------------------------------------------------------------------------------------------
// // ----------------------------------------------------------------------------------------------------
//
//
//
// class RefCountedBase {
//  public:
//   RefCountedBase(const RefCountedBase&) = delete;
//   RefCountedBase& operator=(const RefCountedBase&) = delete;
//   bool HasOneRef() const { return ref_count_ == 1; }
//   bool HasAtLeastOneRef() const { return ref_count_ >= 1; }
//  protected:
//   explicit RefCountedBase(StartRefCountFromZeroTag) {
//   }
//   explicit RefCountedBase(StartRefCountFromOneTag) : ref_count_(1) {
//   }
//   ~RefCountedBase() {
//   }
//   void AddRef() const {
//     AddRefImpl();
//   }
//   // Returns true if the object should self-delete.
//   bool Release() const {
//     ReleaseImpl();
//     return ref_count_ == 0;
//   }
//   // Returns true if it is safe to read or write the object, from a thread
//   // safety standpoint. Should be DCHECK'd from the methods of RefCounted
//   // classes if there is a danger of objects being shared across threads.
//   //
//   // This produces fewer false positives than adding a separate SequenceChecker
//   // into the subclass, because it automatically detaches from the sequence when
//   // the reference count is 1 (and never fails if there is only one reference).
//   //
//   // This means unlike a separate SequenceChecker, it will permit a singly
//   // referenced object to be passed between threads (not holding a reference on
//   // the sending thread), but will trap if the sending thread holds onto a
//   // reference, or if the object is accessed from multiple threads
//   // simultaneously.
//   bool IsOnValidSequence() const {
//   }
//  private:
//   template <typename U>
//   friend scoped_refptr<U> AdoptRef(U*);
//   friend class RefCountedOverflowTest;
//   void Adopted() const {
//   }
// #if defined(ARCH_CPU_64_BITS)
//   void AddRefImpl() const;
//   void ReleaseImpl() const;
// #else
//   void AddRefImpl() const { ++ref_count_; }
//   void ReleaseImpl() const { --ref_count_; }
// #endif
//   mutable uint32_t ref_count_ = 0;
//   static_assert(std::is_unsigned_v<decltype(ref_count_)>,
//                 "ref_count_ must be an unsigned type.");
// };
//
//
//
// class AtomicRefCount {
// private:
//     std::atomic<int> ref_count; // Atomic integer to track reference count
//
// public:
//     // Constructor: Initialize with a specific count (default is 0)
//     explicit AtomicRefCount(int initial_count = 0) : ref_count(initial_count) {}
//
//     // Increment the reference count and return the new value
//     int Increment() {
//         return ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
//     }
//
//     // Decrement the reference count and return true if it reaches zero
//     bool Decrement() {
//         int new_count = ref_count.fetch_sub(1, std::memory_order_acq_rel) - 1;
//         assert(new_count >= 0 && "Reference count should not be negative!");
//         return new_count == 0;
//     }
//
//     // Get the current reference count
//     int GetCount() const {
//         return ref_count.load(std::memory_order_relaxed);
//     }
//
//     // Reset the reference count to a specific value
//     void Reset(int value = 0) {
//         ref_count.store(value, std::memory_order_relaxed);
//     }
// };
//
//
//
// class RefCountedThreadSafeBase {
//  public:
//   RefCountedThreadSafeBase(const RefCountedThreadSafeBase&) = delete;
//   RefCountedThreadSafeBase& operator=(const RefCountedThreadSafeBase&) = delete;
//   bool HasOneRef() const;
//   bool HasAtLeastOneRef() const;
//  protected:
//   explicit  RefCountedThreadSafeBase(StartRefCountFromZeroTag) {}
//   explicit  RefCountedThreadSafeBase(StartRefCountFromOneTag)
//       : ref_count_(1) {
//   }
// // Release and AddRef are suitable for inlining on X86 because they generate
// // very small code sequences.
// //
// // ARM64 devices supporting ARMv8.1-A atomic instructions generate very little
// // code, e.g. fetch_add() with acquire ordering is a single instruction (ldadd),
// // vs LL/SC in previous ARM architectures. Inline it there as well.
// //
// // On other platforms (e.g. ARM), it causes a size regression and is probably
// // not worth it.
// #if defined(ARCH_CPU_X86_FAMILY) || defined(__ARM_FEATURE_ATOMICS)
//   // Returns true if the object should self-delete.
//   bool Release() const { return ReleaseImpl(); }
//   void AddRef() const { AddRefImpl(); }
//   void AddRefWithCheck() const { AddRefWithCheckImpl(); }
// #else
//   // Returns true if the object should self-delete.
//   bool Release() const;
//   void AddRef() const;
//   void AddRefWithCheck() const;
// #endif
//  private:
//   template <typename U>
//   friend scoped_refptr<U> AdoptRef(U*);
//   friend class RefCountedOverflowTest;
//   void Adopted() const {
//   }
//   void AddRefImpl() const {
//     //CHECK_NE(ref_count_.Increment(), std::numeric_limits<int>::max());
//   }
//   void AddRefWithCheckImpl() const {
//     int pre_increment_count = ref_count_.Increment();
//     //CHECK_GT(pre_increment_count, 0);
//     //CHECK_NE(pre_increment_count, std::numeric_limits<int>::max());
//   }
//   bool ReleaseImpl() const {
//     if (!ref_count_.Decrement()) {
//       return true;
//     }
//     return false;
//   }
//   mutable AtomicRefCount ref_count_{0};
// };
//
// // Forward declaration.
// template <class T, typename Traits> class RefCountedThreadSafe;
//
// template <typename T>
// struct DefaultRefCountedThreadSafeTraits {	
//   static void Destruct(const T* x) {
//     // Delete through RefCountedThreadSafe to make child classes only need to be
//     // friend with RefCountedThreadSafe instead of this struct, which is an
//     // implementation detail.
//     RefCountedThreadSafe<T, DefaultRefCountedThreadSafeTraits>::DeleteInternal(x);
//   }
// };
//
//
// template <class T, typename Traits = DefaultRefCountedThreadSafeTraits<T>>
// class RefCountedThreadSafe : public RefCountedThreadSafeBase {
//  public:
//   using RefCountPreferenceTag = StartRefCountFromZeroTag;
//   explicit RefCountedThreadSafe()
//       : RefCountedThreadSafeBase(GetRefCountPreference<T>()) {}
//   RefCountedThreadSafe(const RefCountedThreadSafe&) = delete;
//   RefCountedThreadSafe& operator=(const RefCountedThreadSafe&) = delete;
//   void AddRef() const { AddRefImpl(GetRefCountPreference<T>()); }
//   void Release() const {
//     if (RefCountedThreadSafeBase::Release()) {
//       Traits::Destruct(static_cast<const T*>(this));
//     }
//   }
//  protected:
//   ~RefCountedThreadSafe() = default;
//  private:
//   friend struct DefaultRefCountedThreadSafeTraits<T>;
//   template <typename U>
//   static void DeleteInternal(const U* x) {
//     delete x;
//   }
//   void AddRefImpl(StartRefCountFromZeroTag) const {
//     RefCountedThreadSafeBase::AddRef();
//   }
//   void AddRefImpl(StartRefCountFromOneTag) const {
//     RefCountedThreadSafeBase::AddRefWithCheck();
//   }
// };
//
//
//
//
//
//
// template <typename T>
// class HeapArray {
// private:
//     T* data_;       // Pointer to the dynamically allocated array
//     size_t size_;   // Size of the array
//
// public:
//     // Constructor: Allocates a heap array of the given size
//     explicit HeapArray(size_t size)
//         : data_(new T[size]()), size_(size) {} // Default initialize elements
//
//     // Destructor: Releases the allocated memory
//     ~HeapArray() {
//         delete[] data_;
//     }
//
//     // Disable copy semantics to prevent accidental deep copies
//     HeapArray(const HeapArray&) = delete;
//     HeapArray& operator=(const HeapArray&) = delete;
//
//     // Enable move semantics for efficiency
//     HeapArray(HeapArray&& other) noexcept
//         : data_(other.data_), size_(other.size_) {
//         other.data_ = nullptr;
//         other.size_ = 0;
//     }
//
//     HeapArray& operator=(HeapArray&& other) noexcept {
//         if (this != &other) {
//             delete[] data_;
//             data_ = other.data_;
//             size_ = other.size_;
//             other.data_ = nullptr;
//             other.size_ = 0;
//         }
//         return *this;
//     }
//
//     // Access element at index (unchecked)
//     T& operator[](size_t index) {
//         return data_[index];
//     }
//
//     const T& operator[](size_t index) const {
//         return data_[index];
//     }
//
//     // Access element at index (bounds-checked)
//     T& at(size_t index) {
//         if (index >= size_) {
//             throw std::out_of_range("Index out of range");
//         }
//         return data_[index];
//     }
//
//     const T& at(size_t index) const {
//         if (index >= size_) {
//             throw std::out_of_range("Index out of range");
//         }
//         return data_[index];
//     }
//
//     // Get the size of the array
//     size_t size() const {
//         return size_;
//     }
//
//     // Iterators for range-based loops and STL algorithms
//     T* begin() {
//         return data_;
//     }
//
//     T* end() {
//         return data_ + size_;
//     }
//
//     const T* begin() const {
//         return data_;
//     }
//
//     const T* end() const {
//         return data_ + size_;
//     }
// };
//
//
//
//
// // template <typename T, typename Enable = void>
// // struct AllowPtrArithmetic : std::false_type {};
//
// struct AllowPtrArithmetic {};
//
//
//
// template <typename T>
// class RefCountedData
//     : public RefCountedThreadSafe<RefCountedData<T>> {
//  public:
//   RefCountedData() : data() {}
//   RefCountedData(const T& in_value) : data(in_value) {}
//   RefCountedData(T&& in_value) : data(std::move(in_value)) {}
//   template <typename... Args>
//   explicit RefCountedData(std::in_place_t, Args&&... args)
//       : data(std::forward<Args>(args)...) {}
//   T data;
//  private:
//   friend class RefCountedThreadSafe<RefCountedData<T>>;
//   ~RefCountedData() = default;
// };
//
// class  IOBuffer : public RefCountedThreadSafe<IOBuffer> {
//  public:
//   // Returns the length from bytes() to the end of the buffer. Many methods that
//   // take an IOBuffer also take a size indicated the number of IOBuffer bytes to
//   // use from the start of bytes(). That number must be no more than the size()
//   // of the passed in IOBuffer.
//   int size() const { return size_; }
//   char* data() { return data_; }
//   const char* data() const { return data_; }
//   uint8_t* bytes() { return reinterpret_cast<uint8_t*>(data()); }
//   const uint8_t* bytes() const {
//     return reinterpret_cast<const uint8_t*>(data());
//   }
//   std::span<uint8_t> span() {
//     return (std::span(bytes(), static_cast<size_t>(size_)));
//   }
//   std::span<const uint8_t> span() const {
//     return (std::span(bytes(), static_cast<size_t>(size_)));
//   }
//  protected:
//   friend class RefCountedThreadSafe<IOBuffer>;
//   static void AssertValidBufferSize(size_t size);
//   IOBuffer();
//   explicit IOBuffer(std::span<char> data);
//   explicit IOBuffer(std::span<uint8_t> data);
//   virtual ~IOBuffer();
//   raw_ptr<char, AllowPtrArithmetic> data_ = nullptr;
//   int size_ = 0;
// };
// // Class which owns its buffer and manages its destruction.
// class IOBufferWithSize : public IOBuffer {
//  public:
//   IOBufferWithSize();
//   explicit IOBufferWithSize(size_t size);
//  protected:
//   ~IOBufferWithSize() override;
//  private:
//   HeapArray<char> storage_;
// };
// // This is like IOBufferWithSize, except its constructor takes a vector.
// // IOBufferWithSize uses a HeapArray instead of a vector so that it can avoid
// // initializing its data. VectorIOBuffer is primarily useful useful for writing
// // data, while IOBufferWithSize is primarily useful for reading data.
// class VectorIOBuffer : public IOBuffer {
//  public:
//    VectorIOBuffer(std::vector<uint8_t> vector);
//    //VectorIOBuffer(std::span<uint8_t> std::span);
//  private:
//   ~VectorIOBuffer() override;
//   std::vector<uint8_t> vector_;
// };
// // This is a read only IOBuffer.  The data is stored in a string and
// // the IOBuffer interface does not provide a proper way to modify it.
// class StringIOBuffer : public IOBuffer {
//  public:
//   explicit StringIOBuffer(std::string s);
//  private:
//   ~StringIOBuffer() override;
//   std::string string_data_;
// };
//
//


// //class IOBuffer : public RefCountedThreadSafe<IOBuffer> {
// class IOBuffer  {
// public:
// 	int size() const { return size_; }
//
// 	char* data() { return data_; }
// 	const char* data() const { return data_; }
//
// 	uint8_t* bytes() { return reinterpret_cast<uint8_t*>(data()); }
// 	const uint8_t* bytes() const {
// 		return reinterpret_cast<const uint8_t*>(data());
// 	}
//
// 	std::span<uint8_t> span() {
// 		return std::span(bytes(), static_cast<size_t>(size_));
// 	}
// 	std::span<const uint8_t> span() const {
// 		return std::span(bytes(), static_cast<size_t>(size_));
// 	}
//
// protected:
// 	//friend class RefCountedThreadSafe<IOBuffer>;
//
// 	static void AssertValidBufferSize(size_t size);
//
// 	IOBuffer();
// 	IOBuffer(std::span<char> data);
// 	IOBuffer(std::span<uint8_t> data);
//
// 	virtual ~IOBuffer();
//
// 	//raw_ptr<char, AllowPtrArithmetic> data_ = nullptr;
// 	char* data_ = nullptr;
// 	int size_ = 0;
// };



//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

#if !defined(DEBUG) && V8_HAS_ATTRIBUTE_ALWAYS_INLINE
# define V8_INLINE inline __attribute__((always_inline))
#elif !defined(DEBUG) && V8_HAS___FORCEINLINE
# define V8_INLINE __forceinline
#else
# define V8_INLINE inline
#endif

template <class T>
class Local {
public:
//	Local() : val_(nullptr) {}
//	template <class S>
//	V8_INLINE Local(Local<S> that)
//		: val_(reinterpret_cast<T*>(*that)) {
//		TYPE_CHECK(T, S);
//	}
//
//	V8_INLINE bool IsEmpty() const { return val_ == nullptr; }
//
//	V8_INLINE void Clear() { val_ = nullptr; }
//
//	V8_INLINE T* operator->() const { return val_; }
//
//	V8_INLINE T* operator*() const { return val_; }
//
//	template <class S>
//	V8_INLINE bool operator==(const Local<S>& that) const {
//		internal::Address* a = reinterpret_cast<internal::Address*>(this->val_);
//		internal::Address* b = reinterpret_cast<internal::Address*>(that.val_);
//		if (a == nullptr) return b == nullptr;
//		if (b == nullptr) return false;
//		return *a == *b;
//	}
//
//	template <class S> V8_INLINE bool operator==(
//		const PersistentBase<S>& that) const {
//		internal::Address* a = reinterpret_cast<internal::Address*>(this->val_);
//		internal::Address* b = reinterpret_cast<internal::Address*>(that.val_);
//		if (a == nullptr) return b == nullptr;
//		if (b == nullptr) return false;
//		return *a == *b;
//	}
//
//	template <class S>
//	V8_INLINE bool operator!=(const Local<S>& that) const {
//		return !operator==(that);
//	}
//
//	template <class S> V8_INLINE bool operator!=(
//		const Persistent<S>& that) const {
//		return !operator==(that);
//	}
//
//	template <class S> V8_INLINE static Local<T> Cast(Local<S> that) {
//#ifdef V8_ENABLE_CHECKS
//		// If we're going to perform the type check then we have to check
//		// that the handle isn't empty before doing the checked cast.
//		if (that.IsEmpty()) return Local<T>();
//#endif
//		return Local<T>(T::Cast(*that));
//	}
//
//	template <class S>
//	V8_INLINE Local<S> As() const {
//		return Local<S>::Cast(*this);
//	}
//
//	V8_INLINE static Local<T> New(Isolate* isolate, Local<T> that);
//	V8_INLINE static Local<T> New(Isolate* isolate,
//		const PersistentBase<T>& that);
//	V8_INLINE static Local<T> New(Isolate* isolate, const TracedGlobal<T>& that);

private:
	/*friend class Utils;
	template<class F> friend class Eternal;
	template<class F> friend class PersistentBase;
	template<class F, class M> friend class Persistent;
	template<class F> friend class Local;
	template <class F>
	friend class MaybeLocal;
	template<class F> friend class FunctionCallbackInfo;
	template<class F> friend class PropertyCallbackInfo;
	friend class String;
	friend class Object;
	friend class Context;
	friend class Isolate;
	friend class Private;
	template<class F> friend class internal::CustomArguments;
	friend Local<Primitive> Undefined(Isolate* isolate);
	friend Local<Primitive> Null(Isolate* isolate);
	friend Local<Boolean> True(Isolate* isolate);
	friend Local<Boolean> False(Isolate* isolate);
	friend class HandleScope;
	friend class EscapableHandleScope;
	template <class F1, class F2, class F3>
	friend class PersistentValueMapBase;
	template<class F1, class F2> friend class PersistentValueVector;
	template <class F>
	friend class ReturnValue;
	template <class F>
	friend class TracedGlobal;*/

	//explicit V8_INLINE Local(T* that) : val_(that) {}
	//V8_INLINE static Local<T> New(Isolate* isolate, T* that);
	T* val_;
};


#if V8_HAS_ATTRIBUTE_WARN_UNUSED_RESULT
#define V8_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define V8_WARN_UNUSED_RESULT /* NOT SUPPORTED */
#endif

template <class T>
class MaybeLocal {
public:
	V8_INLINE MaybeLocal() : local_() {}
	template <class S>
	V8_INLINE MaybeLocal(Local<S> that) : local_(that) {}

	V8_INLINE bool IsEmpty() const { return local_.IsEmpty(); }

	/**
	 * Converts this MaybeLocal<> to a Local<>. If this MaybeLocal<> is empty,
	 * |false| is returned and |out| is assigned with nullptr.
	 */
	template <class S>
	V8_WARN_UNUSED_RESULT V8_INLINE bool ToLocal(Local<S>* out) const {
		*out = local_;
		return !IsEmpty();
	}

	/**
	 * Converts this MaybeLocal<> to a Local<>. If this MaybeLocal<> is empty,
	 * V8 will crash the process.
	 */
	V8_INLINE Local<T> ToLocalChecked() {
		//if (V8_UNLIKELY(IsEmpty())) api_internal::ToLocalEmpty(); // NOTE: alf
		return local_;
	}

	/**
	 * Converts this MaybeLocal<> to a Local<>, using a default value if this
	 * MaybeLocal<> is empty.
	 */
	template <class S>
	V8_INLINE Local<S> FromMaybe(Local<S> default_value) const {
		return IsEmpty() ? default_value : Local<S>(local_);
	}

	/**
	 * Cast a handle to a subclass, e.g. MaybeLocal<Value> to MaybeLocal<Object>.
	 * This is only valid if the handle actually refers to a value of the target
	 * type.
	 */
	template <class S>
	V8_INLINE static MaybeLocal<T> Cast(MaybeLocal<S> that) {
#ifdef V8_ENABLE_CHECKS
		// If we're going to perform the type check then we have to check
		// that the handle isn't empty before doing the checked cast.
		if (that.IsEmpty()) return MaybeLocal<T>();
		T::Cast(that.local_.template value<S>());
#endif
		return MaybeLocal<T>(that.local_);
	}

	/**
	 * Calling this is equivalent to MaybeLocal<S>::Cast().
	 * In particular, this is only valid if the handle actually refers to a value
	 * of the target type.
	 */
	template <class S>
	V8_INLINE MaybeLocal<S> As() const {
		return MaybeLocal<S>::Cast(*this);
	}

private:
	Local<T> local_;

	template <typename S>
	friend class MaybeLocal;
};
