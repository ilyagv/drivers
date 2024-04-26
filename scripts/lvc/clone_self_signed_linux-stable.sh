#!/bin/bash

REPO=http://git.linuxtesting.ru/pub/scm/linux/kernel/git/lvc/linux-stable.git

# Disable SSL verification to clone repository that uses self signed certificate
git -c http.sslVerify=false clone ${REPO}

