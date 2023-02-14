#!/bin/bash

d=99
for i in {8..27}; do echo -n "$i: "; date; /usr/bin/time -v ./benchmark_denseqmc $i ${d}; echo; done |& tee logs/benchmark_dense_${d}.log

d=1
for i in {8..31}; do echo -n "$i: "; date; /usr/bin/time -v ./benchmark_sparseqmc $i ${d}; echo; done |& tee logs/benchmark_sparse_${d}.log

d=25
for i in {8..31}; do echo -n "$i: "; date; /usr/bin/time -v ./benchmark_sparseqmc $i ${d}; echo; done |& tee logs/benchmark_sparse_${d}.log

d=99
for i in {8..22}; do echo -n "$i: "; date; /usr/bin/time -v ./benchmark_sparseqmc $i ${d}; echo; done |& tee logs/benchmark_sparse_${d}.log

d=100
for i in {8..22}; do echo -n "$i: "; date; /usr/bin/time -v ./benchmark_sparseqmc $i ${d}; echo; done |& tee logs/benchmark_sparse_${d}.log

d=75
for i in {8..27}; do echo -n "$i: "; date; /usr/bin/time -v ./benchmark_sparseqmc $i ${d}; echo; done |& tee logs/benchmark_sparse_${d}.log

d=50
for i in {8..30}; do echo -n "$i: "; date; /usr/bin/time -v ./benchmark_sparseqmc $i ${d}; echo; done |& tee logs/benchmark_sparse_${d}.log