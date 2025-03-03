# evnotif

event notifier for IRC

## What does it do?

It simply connects to IRC, keeps alive, reads a line from FIFO, and sends it to IRC.

You write to FIFO from like Webhook backend to like send a commit log to IRC.

## How do I build it?

Simply run `make`.
