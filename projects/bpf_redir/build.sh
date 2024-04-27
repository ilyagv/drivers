#!/bin/bash

clang -O2 -g -target bpf -c bpf_sockops_v4.c -o bpf_sockops_v4.o
clang -O2 -g -Wall -target bpf -c bpf_tcpip_bypass.c -o bpf_tcpip_bypass.o
