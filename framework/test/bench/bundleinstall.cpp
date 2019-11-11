#include <chrono>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleEvent.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <thread>

#include "TestUtils.h"
#include "benchmark/benchmark.h"

class BundleInstallFixture : public ::benchmark::Fixture
{
public:
  using benchmark::Fixture::SetUp;
  using benchmark::Fixture::TearDown;

  void SetUp(const ::benchmark::State&) {}

  ~BundleInstallFixture() = default;

protected:
  void InstallWithCppFramework(benchmark::State& state,
                               const std::string& bundleName)
  {
    using namespace std::chrono;
    using namespace cppmicroservices;

    auto framework = cppmicroservices::FrameworkFactory().NewFramework();
    framework.Start();
    auto context = framework.GetBundleContext();
    for (auto _ : state) {
      auto start = high_resolution_clock::now();
      auto bundle = testing::InstallLib(context, bundleName);
      auto end = high_resolution_clock::now();
      auto elapsed = duration_cast<duration<double>>(end - start);
      state.SetIterationTime(elapsed.count());
#ifdef US_BUILD_SHARED_LIBS
      bundle.Uninstall();
#endif
    }

    framework.Stop();
    framework.WaitForStop(std::chrono::milliseconds::zero());
  }
};

BENCHMARK_DEFINE_F(BundleInstallFixture, BundleInstallCppFramework)
(benchmark::State& state)
{
  InstallWithCppFramework(state, "dummyService");
}

BENCHMARK_DEFINE_F(BundleInstallFixture, LargeBundleInstallCppFramework)
(benchmark::State& state)
{
  InstallWithCppFramework(state, "largeBundle");
}

BENCHMARK_DEFINE_F(BundleInstallFixture, ConcurrentBundleInstallOldCppFramework)
(benchmark::State& state)
{
  using namespace std::chrono;
  using namespace cppmicroservices;

  auto framework = cppmicroservices::FrameworkFactory().NewFramework();
  framework.Start();
  auto context = framework.GetBundleContext();

  const std::string bundlesToInstall[10] = { "TestBundleA", "TestBundleA2",
                                             "TestBundleB", "TestBundleC1",
                                             "TestBundleH", "TestBundleS",
                                             "TestBundleR", "TestBundleLQ",
                                             "TestBundleA", "TestBundleB" };

  for (auto _ : state) {
    std::thread threads[10];
    auto start = high_resolution_clock::now();
    for (int i = 0; i < 10; i++) {
      threads[i] =
        std::thread(testing::InstallLib, context, bundlesToInstall[i]);
    }
    for (int i = 0; i < 10; i++) {
      threads[i].join();
    }
    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<duration<double>>(end - start);
    state.SetIterationTime(elapsed.count());
  }

  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());
}

BENCHMARK_DEFINE_F(BundleInstallFixture, ConcurrentBundleInstallNewCppFramework)
(benchmark::State& state)
{
  using namespace std::chrono;
  using namespace cppmicroservices;

  auto framework = cppmicroservices::FrameworkFactory().NewFramework();
  framework.Start();
  auto context = framework.GetBundleContext();

  const std::string bundlesToInstall[10] = { "TestBundleA", "TestBundleA2",
                                             "TestBundleB", "TestBundleC1",
                                             "TestBundleH", "TestBundleS",
                                             "TestBundleR", "TestBundleLQ",
                                             "TestBundleA", "TestBundleB" };

  for (auto _ : state) {
    std::thread threads[10];
    auto start = high_resolution_clock::now();
    for (int i = 0; i < 10; i++) {
      threads[i] =
        std::thread(testing::InstallLibNew, context, bundlesToInstall[i]);
    }
    for (int i = 0; i < 10; i++) {
      threads[i].join();
    }
    auto end = high_resolution_clock::now();
    auto elapsed = duration_cast<duration<double>>(end - start);
    state.SetIterationTime(elapsed.count());
  }

  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());
}

// Register functions as benchmark
BENCHMARK_REGISTER_F(BundleInstallFixture, BundleInstallCppFramework)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture, LargeBundleInstallCppFramework)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture,
                     ConcurrentBundleInstallOldCppFramework)
  ->UseManualTime();
BENCHMARK_REGISTER_F(BundleInstallFixture,
                     ConcurrentBundleInstallNewCppFramework)
  ->UseManualTime();