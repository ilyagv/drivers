#!/bin/bash

REPO=https://git.linuxtesting.ru/private/scm/linux/kernel/git/lvc/linux-stable-lvc.git

# Disable SSL verification to clone repository that uses self signed certificate
git -c http.sslVerify=false clone ${REPO}

