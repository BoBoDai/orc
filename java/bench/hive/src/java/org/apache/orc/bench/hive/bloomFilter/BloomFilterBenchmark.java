package org.apache.orc.bench.hive.bloomFilter;

import com.google.auto.service.AutoService;
import org.apache.orc.bench.core.OrcBenchmark;
import org.apache.orc.util.BloomFilter;
import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;

import java.nio.charset.StandardCharsets;
import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.TimeUnit;

@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.MICROSECONDS)
@State(Scope.Thread)
@AutoService(OrcBenchmark.class)
public class BloomFilterBenchmark implements OrcBenchmark {

    private static final int BATCH_SIZE = 1024;

    @State(Scope.Thread)
    public static class BenchmarkState {
        @Param({"100000"})
        public int bloomFilterSize;

        @Param({"0.05"})
        public double falsePositiveRate;

        BloomFilter bloomFilter;
        byte[][] data;
        int[] start;
        int[] length;

        Random random;

        @Setup
        public void setup() {
            random = new Random(42);
            bloomFilter = new BloomFilter(bloomFilterSize, falsePositiveRate);

            data = new byte[BATCH_SIZE][];
            start = new int[BATCH_SIZE];
            length = new int[BATCH_SIZE];

            // 生成模拟的 UTF-8 字符串数据（如 user_id_123）
            for (int i = 0; i < BATCH_SIZE; i++) {
                String str = randomString(1000);
                data[i] = str.getBytes(StandardCharsets.UTF_8);
                start[i] = 0;
                length[i] = data[i].length;
            }

            // 预先添加前 50% 到布隆过滤器
            for (int i = 0; i < BATCH_SIZE / 2; i++) {
                bloomFilter.addBytes(data[i], start[i], length[i]);
            }
        }

        public static String randomString(int length) {
            Random random = new Random();
            StringBuilder sb = new StringBuilder(length);
            for (int i = 0; i < length; i++) {
                sb.append((char) ('a' + random.nextInt(26)));
            }
            return sb.toString();
        }
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    @OutputTimeUnit(TimeUnit.NANOSECONDS)
    @OperationsPerInvocation(BATCH_SIZE)
    @Warmup(iterations = 10, time = 2, timeUnit = TimeUnit.SECONDS)
    @Measurement(iterations = 10, time = 2, timeUnit = TimeUnit.SECONDS)
    public void addBytes_FromBytesVector(Blackhole blackhole, BenchmarkState state) {
        BloomFilter bf = new BloomFilter(state.bloomFilterSize, state.falsePositiveRate);
        for (int i = 0; i < BATCH_SIZE; i++) {
            bf.addBytes(state.data[i], state.start[i], state.length[i]);
        }
        blackhole.consume(bf);
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    @OutputTimeUnit(TimeUnit.NANOSECONDS)
    @OperationsPerInvocation(1)
    @Warmup(iterations = 10, time = 2, timeUnit = TimeUnit.SECONDS)
    @Measurement(iterations = 10, time = 2, timeUnit = TimeUnit.SECONDS)
    public void testBytes_RandomLookup(Blackhole blackhole, BenchmarkState state) {
        ThreadLocalRandom random = ThreadLocalRandom.current();
        int idx = random.nextInt(BATCH_SIZE);

        boolean result = state.bloomFilter.testBytes(state.data[idx], state.start[idx], state.length[idx]);
        blackhole.consume(result);
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    @OutputTimeUnit(TimeUnit.NANOSECONDS)
    @OperationsPerInvocation(10_000)
    @Warmup(iterations = 5)
    @Measurement(iterations = 5)
    public void testBytes_10K(Blackhole blackhole, BenchmarkState state) {
        boolean result = false;
        ThreadLocalRandom random = ThreadLocalRandom.current();
        for (int i = 0; i < 10_000; i++) {
            int idx = random.nextInt(BATCH_SIZE);
            result = state.bloomFilter.testBytes(state.data[idx], state.start[idx], state.length[idx]);
        }
        blackhole.consume(result);
    }


    @Override
    public String getName() {
        return "bloom-filter";
    }

    @Override
    public String getDescription() {
        return "Benchmark BloomFilter add and test operations with strings";
    }

    @Override
    public void run(String[] args) throws Exception {
        Options options = new OptionsBuilder()
                .include(BloomFilterBenchmark.class.getSimpleName())
                .warmupIterations(5)
                .measurementIterations(5)
                .forks(1)
                .build();

        new Runner(options).run();
    }
}