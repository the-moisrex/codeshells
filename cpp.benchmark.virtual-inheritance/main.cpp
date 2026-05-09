#include <benchmark/benchmark.h>

// Cross-platform macro to prevent inlining.
#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

// ============================================================================
// Helper to prevent devirtualization by hiding the exact type from the
// optimizer
// ============================================================================
template <typename Base, typename Impl> Base *hide_type(Impl *ptr) {
  // 1. Implicitly or explicitly cast to Base* FIRST to apply MI offsets
  Base *base_ptr = static_cast<Base *>(ptr);

  // 2. Now hide the adjusted pointer from the optimizer
  volatile void *vptr = base_ptr;
  return static_cast<Base *>(const_cast<void *>(vptr));
}

// ============================================================================
// 1. Direct / Non-Virtual (Baseline)
// ============================================================================
struct DirectImpl {
  NOINLINE int op1() { return 1; }
  NOINLINE int op2() { return 2; }
  NOINLINE int op3() { return 3; }
};

// ============================================================================
// 2. Template (CRTP) - Compile-Time Polymorphism
// ============================================================================
template <typename Derived> struct CRTPBase {
  NOINLINE int op1() { return static_cast<Derived *>(this)->op1_impl(); }
  NOINLINE int op2() { return static_cast<Derived *>(this)->op2_impl(); }
  NOINLINE int op3() { return static_cast<Derived *>(this)->op3_impl(); }
};

struct CRTPImpl : CRTPBase<CRTPImpl> {
  NOINLINE int op1_impl() { return 1; }
  NOINLINE int op2_impl() { return 2; }
  NOINLINE int op3_impl() { return 3; }
};

// ============================================================================
// 3. Single Inheritance (Flat)
// ============================================================================
struct FlatBase {
  virtual ~FlatBase() = default;
  virtual int op1() = 0;
  virtual int op2() = 0;
  virtual int op3() = 0;
};

struct FlatImpl : FlatBase {
  NOINLINE int op1() override { return 1; }
  NOINLINE int op2() override { return 2; }
  NOINLINE int op3() override { return 3; }
};

// ============================================================================
// 4. Standard Multiple Inheritance
// ============================================================================
struct MIBase1 {
  virtual ~MIBase1() = default;
  virtual int op1() = 0;
};
struct MIBase2 {
  virtual ~MIBase2() = default;
  virtual int op2() = 0;
};
struct MIBase3 {
  virtual ~MIBase3() = default;
  virtual int op3() = 0;
};

struct MIImpl final : MIBase1, MIBase2, MIBase3 {
  NOINLINE int op1() override { return 1; }
  NOINLINE int op2() override { return 2; }
  NOINLINE int op3() override { return 3; }
};

struct MIBasePrivate : protected MIBase1, protected MIBase2, protected MIBase3 {

  NOINLINE int op12() { return op1(); }
  NOINLINE int op22() { return op2(); }
  NOINLINE int op32() { return op3(); }
};

struct MIImplPrivate final : MIBasePrivate {
private:
  NOINLINE int op1() override { return 1; }
  NOINLINE int op2() override { return 2; }
  NOINLINE int op3() override { return 3; }

public:
  NOINLINE int op12() { return op1(); }
  NOINLINE int op22() { return op2(); }
  NOINLINE int op32() { return op3(); }
};
// ============================================================================
// 5. Templated Multiple Inheritance (Mixins)
// ============================================================================
template <typename B1, typename B2, typename B3>
struct TemplatedMIImpl : B1, B2, B3 {
  NOINLINE int op1() override { return 1; }
  NOINLINE int op2() override { return 2; }
  NOINLINE int op3() override { return 3; }
};

template <typename B1, typename B2, typename B3>
struct TemplatedMIImpl2 : virtual B1, virtual B2, virtual B3 {
  NOINLINE int op1() override { return 1; }
  NOINLINE int op2() override { return 2; }
  NOINLINE int op3() override { return 3; }
};

// ============================================================================
// 6. Deep Inheritance (Overrides a class that inherits templated bases)
// ============================================================================
struct DeepMIImpl : TemplatedMIImpl<MIBase1, MIBase2, MIBase3> {
  NOINLINE int op1() override { return 11; }
  NOINLINE int op2() override { return 22; }
  NOINLINE int op3() override { return 33; }
};

struct DeepMIImpl2 final : TemplatedMIImpl2<MIBase1, MIBase2, MIBase3> {
  NOINLINE int op1() override { return 11; }
  NOINLINE int op2() override { return 22; }
  NOINLINE int op3() override { return 33; }
};

// struct DeepMIImpl3 final : private TemplatedMIImpl2<MIBase1, MIBase2,
// MIBase3> { private:
//   NOINLINE int op1() override { return 11; }
//   NOINLINE int op2() override { return 22; }
//   NOINLINE int op3() override { return 33; }

// public:
//   MIBase1 *get_b1() { return hide_type<MIBase1>(this); }

//   MIBase2 *get_b2() { return hide_type<MIBase2>(this); }

//   MIBase3 *get_b3() { return hide_type<MIBase3>(this); }

//   NOINLINE int op12() { return op1(); }
//   NOINLINE int op22() { return op2(); }
//   NOINLINE int op32() { return op3(); }
// };

// ============================================================================
// 7. Virtual Multiple Inheritance (The Diamond Problem structure)
// ============================================================================
struct VBase1 {
  virtual ~VBase1() = default;
  virtual int op1() = 0;
};
struct VBase2 {
  virtual ~VBase2() = default;
  virtual int op2() = 0;
};
struct VBase3 {
  virtual ~VBase3() = default;
  virtual int op3() = 0;
};

struct VMIImpl : virtual VBase1, virtual VBase2, virtual VBase3 {
  NOINLINE int op1() override { return 1; }
  NOINLINE int op2() override { return 2; }
  NOINLINE int op3() override { return 3; }
};

// ============================================================================
// Benchmarks
// ============================================================================

static void BM_DirectCall(benchmark::State &state) {
  DirectImpl impl;
  DirectImpl *ptr = &impl;
  benchmark::DoNotOptimize(ptr);

  for (auto _ : state) {
    benchmark::DoNotOptimize(ptr->op1());
    benchmark::DoNotOptimize(ptr->op2());
    benchmark::DoNotOptimize(ptr->op3());
  }
}
BENCHMARK(BM_DirectCall);

static void BM_TemplateCRTP(benchmark::State &state) {
  CRTPImpl impl;
  CRTPBase<CRTPImpl> *ptr = &impl;
  benchmark::DoNotOptimize(ptr);

  for (auto _ : state) {
    benchmark::DoNotOptimize(ptr->op1());
    benchmark::DoNotOptimize(ptr->op2());
    benchmark::DoNotOptimize(ptr->op3());
  }
}
BENCHMARK(BM_TemplateCRTP);

static void BM_SingleInheritance(benchmark::State &state) {
  FlatImpl impl;
  FlatBase *base = hide_type<FlatBase>(&impl);
  benchmark::DoNotOptimize(base);

  for (auto _ : state) {
    benchmark::DoNotOptimize(base->op1());
    benchmark::DoNotOptimize(base->op2());
    benchmark::DoNotOptimize(base->op3());
  }
}
BENCHMARK(BM_SingleInheritance);

static void BM_MultipleInheritance(benchmark::State &state) {
  MIImpl impl;
  MIBase1 *b1 = hide_type<MIBase1>(&impl);
  MIBase2 *b2 = hide_type<MIBase2>(&impl);
  MIBase3 *b3 = hide_type<MIBase3>(&impl);
  benchmark::DoNotOptimize(b1);
  benchmark::DoNotOptimize(b2);
  benchmark::DoNotOptimize(b3);

  for (auto _ : state) {
    benchmark::DoNotOptimize(b1->op1());
    benchmark::DoNotOptimize(b2->op2());
    benchmark::DoNotOptimize(b3->op3());
  }
}
BENCHMARK(BM_MultipleInheritance);

static void BM_MultipleInheritanceProtected(benchmark::State &state) {
  MIImplPrivate impl;
  auto *base = hide_type<MIBasePrivate>(&impl);
  benchmark::DoNotOptimize(base);

  for (auto _ : state) {
    benchmark::DoNotOptimize(base->op12());
    benchmark::DoNotOptimize(base->op22());
    benchmark::DoNotOptimize(base->op32());
  }
}
BENCHMARK(BM_MultipleInheritanceProtected);

static void BM_TemplatedMultipleInheritance(benchmark::State &state) {
  TemplatedMIImpl<MIBase1, MIBase2, MIBase3> impl;
  MIBase1 *b1 = hide_type<MIBase1>(&impl);
  MIBase2 *b2 = hide_type<MIBase2>(&impl);
  MIBase3 *b3 = hide_type<MIBase3>(&impl);
  benchmark::DoNotOptimize(b1);
  benchmark::DoNotOptimize(b2);
  benchmark::DoNotOptimize(b3);

  for (auto _ : state) {
    benchmark::DoNotOptimize(b1->op1());
    benchmark::DoNotOptimize(b2->op2());
    benchmark::DoNotOptimize(b3->op3());
  }
}
BENCHMARK(BM_TemplatedMultipleInheritance);

static void BM_DeepTemplatedMultipleInheritance(benchmark::State &state) {
  DeepMIImpl impl;
  MIBase1 *b1 = hide_type<MIBase1>(&impl);
  MIBase2 *b2 = hide_type<MIBase2>(&impl);
  MIBase3 *b3 = hide_type<MIBase3>(&impl);
  benchmark::DoNotOptimize(b1);
  benchmark::DoNotOptimize(b2);
  benchmark::DoNotOptimize(b3);

  for (auto _ : state) {
    benchmark::DoNotOptimize(b1->op1());
    benchmark::DoNotOptimize(b2->op2());
    benchmark::DoNotOptimize(b3->op3());
  }
}
BENCHMARK(BM_DeepTemplatedMultipleInheritance);

static void BM_DeepTemplatedMultipleInheritance2(benchmark::State &state) {
  DeepMIImpl2 impl;
  MIBase1 *b1 = hide_type<MIBase1>(&impl);
  MIBase2 *b2 = hide_type<MIBase2>(&impl);
  MIBase3 *b3 = hide_type<MIBase3>(&impl);
  benchmark::DoNotOptimize(b1);
  benchmark::DoNotOptimize(b2);
  benchmark::DoNotOptimize(b3);

  for (auto _ : state) {
    benchmark::DoNotOptimize(b1->op1());
    benchmark::DoNotOptimize(b2->op2());
    benchmark::DoNotOptimize(b3->op3());
  }
}
BENCHMARK(BM_DeepTemplatedMultipleInheritance2);

// static void
// BM_DeepTemplatedMultipleInheritance3Private(benchmark::State &state) {
//   DeepMIImpl3 impl;
//   MIBase1 *b1 = impl.get_b1();
//   MIBase2 *b2 = impl.get_b2();
//   MIBase3 *b3 = impl.get_b3();
//   benchmark::DoNotOptimize(b1);
//   benchmark::DoNotOptimize(b2);
//   benchmark::DoNotOptimize(b3);

//   for (auto _ : state) {
//     benchmark::DoNotOptimize(b1->op1());
//     benchmark::DoNotOptimize(b2->op2());
//     benchmark::DoNotOptimize(b3->op3());
//   }
// }
// BENCHMARK(BM_DeepTemplatedMultipleInheritance3Private);

static void BM_VirtualInheritance(benchmark::State &state) {
  VMIImpl impl;
  VBase1 *b1 = hide_type<VBase1>(&impl);
  VBase2 *b2 = hide_type<VBase2>(&impl);
  VBase3 *b3 = hide_type<VBase3>(&impl);
  benchmark::DoNotOptimize(b1);
  benchmark::DoNotOptimize(b2);
  benchmark::DoNotOptimize(b3);

  for (auto _ : state) {
    benchmark::DoNotOptimize(b1->op1());
    benchmark::DoNotOptimize(b2->op2());
    benchmark::DoNotOptimize(b3->op3());
  }
}
BENCHMARK(BM_VirtualInheritance);
