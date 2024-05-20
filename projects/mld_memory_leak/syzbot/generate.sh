#!/bin/bash

#{"threaded":false,"repeat":true,"procs":1,"slowdown":1,"sandbox":"none","sandbox_arg":0,"leak":true,"netdev":true,"close_fds":true}
#
##{"repeat":true,"procs":1,"slowdown":1,"sandbox":"none","leak":true,"tun":true,"netdev":true,"cgroups":true,"binfmt_misc":true,"close_fds":true,"wifi":true,"sysctl":true,"tmpdir":true,"segv":true}

#
./syz-prog2c -prog ./repro_hub1.syz -repeat -1 -procs 1 -slowdown 1 -sandbox none -sandbox_arg 0 -leak -enable "netdev,close_fds" -trace > repro1.c
#./syz-prog2c -prog ./repro_hub1.syz -threaded -repeat -1 -procs 1 -slowdown 1 -sandbox none -sandbox_arg 0 -leak -enable "net_dev,close_fds" -trace > repro1.c
#./syz-prog2c -prog ./repro1.syz -repeat -1 -procs 1 -slowdown 1 -sandbox none -sandbox_arg 0 -leak -enable "net_dev,close_fds" -trace > repro1.c

