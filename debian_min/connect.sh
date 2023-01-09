#!/usr/bin/env bash

ssh -i ./bullseye.id_rsa -p 10021 -o "StrictHostKeyChecking no" root@localhost

