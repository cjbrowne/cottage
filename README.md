<!-- todo: add screenshot -->

[![Discord](https://img.shields.io/discord/1168975846774681641)](https://discord.gg/g56RMyySgt)
![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/cjbrowne/hopper/nightly.yml)
![GitHub last commit (branch)](https://img.shields.io/github/last-commit/cjbrowne/hopper/develop)
![GitHub all releases](https://img.shields.io/github/downloads/cjbrowne/hopper/total)

# Hopper

Hopper is an experimental operating system which features a kernel-space web 
server.  The goal is to create an OS whose userspace is created primarily using
JavaScript running in a browser.  This simplifies application development
dramatically, as application authors get access to everything a browser has to
offer in terms of UI components.

See [TODO](TODO.md) for how much of the vision has been implemented so far, it's
meant as a rough "roadmap" of what I'm working on.

If you want to watch live development happen, I am often in the discord streaming 
live while I write code.  Feel free to pop in and say hi!

# Running

To run a downloaded ISO image using qemu, run the following command:

`qemu-system-x86_64 -M q35 -m 2G -bios $(OVMF_IMAGE) -cdrom $(IMAGE_NAME).iso -boot d`

The OVMF image is up to you to provide, but usually can be found at:

`/usr/share/ovmf/OVMF.fd`

If it's not there, make sure you have the `ovmf` package from your distribution
vendor installed.

Running on Windows is not supported, though you're welcome to give it a go.

If you're brave enough to try it on real hardware, please post your results in
the "show and tell" discussion forum, or on the discord!

# Design Philosophy

This OS favours simplicity and stability over broad feature support, and will
only implement as much as it strictly needs to in order to get the job done.

Web-first design enables us to do away with some pesky stuff like handling
keyboard and mouse input ourselves (the browser can handily convert them to
network packets for us), and also abstract away a lot of display technology 
since again, we're just rendering into a framebuffer and then sending that to 
someone else's software (the browser, again, and the OS its running on) and 
letting that software do the heavy lifting of working out resolutions, bitmap
depth, and all that fancy jazz.

Clean, simple code that does the job and moves on is preferred over messy tangles.

The kernel is modular by design, and the modules follow a linux-y model of 
exposing (block) devices into the virtual filesystem, though neither ABI nor
API compatibility with linux is a goal here.

We also prefer to implement as much of the OS in kernel modules or userspace as
possible, rather than implementing stuff directly into the kernel.  This is to 
keep the kernel itself lightweight and versatile, and also to prevent tying the
kernel to a particular piece of hardware (as we always will be testing the 
kernel with zero modules loaded, so that no module becomes a strict dependency).

# Development Methodology

We'll use github CI to produce ISO images for download from tagged releases,
which will occur whenever I feel like tagging and releasing.  A nightly build
may also be made available somehow, though you're welcome to build from source.

HDD images won't be produced automatically, you'll need to build these yourself.

All development happens on trunk (master/main/trunk), branches can be made from 
tagged releases if patches or backports are needed.

PRs are welcome, if you see something that could be improved then feel free to 
send me a PR and I'll take a look at it.

We'll set up a discord if anyone gets interested enough to want to join the 
project.

# Syscall HTTP Protocol

TBD

# User Interface

TBD
