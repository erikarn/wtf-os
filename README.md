wtf-os
======

# What the Heck OS

This is a work in progress to build a little toy operating system for
the cortex M4 series hardware.

It's targeting the STM32F429 on the discovery board, but with some BSP
work and bootstrap code it should run on any other cortex M4 device.

# What are you trying to do?

* Build a small footprint operating system for Cortex-M4 and similar SoCs.
* Focus on size, power, efficiency and security.
* Yes, do it all in C, for reasons.
* Support kernel and user tasks.
* Support XIP and run from memory execution of user tasks.
* Support some form of kernel and user libraries, both XIP and from memory.
* Use the ARMv7 MPU to limit what userland tasks can get away with, or
  not limit what they can get away with.
* Try to build out a model that supports drivers being either in userland
  or kernel, and have build time options map that to async RPC or direct
  function calls.
* Be able to do things like turn off SDRAM pools and ensure that tasks
  that are using it are cleanly exited (yes, to be able to power /off/ SDRAM
  banks during low power modes.)
* Yeah, build an OS that has people asking "what the heck adrian?"

# Building

* grab the armv7 eabi hardfpu toolchain that's used for building
  stuff targeting these SoCs directly
* hook up an STM32F429-DISC board via USB
* cd src && make && make flash
* look at the serial IO

# What's supported so far?

* Kernel tasks - XIP (although non-XIP is easy to do)
* User tasks w/ MPU - XIP (again, non-XIP is easy to do)
* SDRAM controller
* MPU support - yes userland tasks are MPU protected!
* GPIO, Serial, clock, power tree
* NVIC
* System Timer
* Configurable event logging
* Yes, and a mini printf()

# What's next?

(Ie, what am I actively working on in my tree right now.)

* Basic async IPC (copying, but non-blocking, will look at zero-copy
  pipelines later for things that support it.)
* LCD / display controller
* I2C controller

# I want to contribute!

Great! Drop me a line. <adrian.chadd@gmail.com> , or open up a pull request
and/or issue!
