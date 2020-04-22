    KeyboardMouse User Guide
    ------------------------

This is a small utility program for Linux to map keyboard key combinations to mouse movement and button clicks.  It should
work on any modern Linux system.

This is a user-space program using the Linux /dev/input file system.  You tell it which event device file under /dev/input to
listen to and it will create a second event device file under /dev/input with a modified stream of events that translates
some combinations of keyboard events into mouse events instead.

It's a simple, single .c file.  It should work on any modern Linux system.

To build, run

    gcc -Wall -o keyboard_mouse keyboard_mouse.c

Here's the command I use to run it:

    sudo ./keyboard_mouse /dev/input/by-id/usb-Kinesis_Advantage2_Keyboard_314159265359-if01-event-kbd

On your system, you'll want to use a different /dev/input device.  Just choose the one corresponding to the keyboard you want
to add mouse actions to.  The sudo part is so it has the permissions it needs for /dev/input.

When you want to remove the keyboard mapping, just kill the keyboard_mouse process.

The program grabs exclusive use of the device it's reading from, so that other readers of that device don't get any more
events from it.  This works out well with X11 because X11 notices the new device and starts interpretting the new device as a
new keyboard/mouse.  It keeps listening to the original keyboard device, but it doesn't see any more keystrokes from the
original until keyboard_mouse exits, then it starts seeing the keystrokes on the original device again.

This program implements the following behavior:

  * The right-meta key (on my Kinesis Advantage 2 it's the key marked with the Windows logo) in combination with W, E, R, S,
    F, X, C, or V moves the mouse.  The direction of movement is the direction of the key relative to the D key -- so, up and
    to the left for W, up for E, up and to the right for R, etc.  The amount of movement is 10 pixels for a single keystroke.
    If the key is left depressed, it will be repeated by the keyboard and at each repeat the magnitude of the movement will
    increase by 1 pixel.  So, the down action will move it by 10, the first repeat will move it by 11 more, the second repeat
    by 12 more, and so on.  This acceleration makes it relatively fast to go long distances while then allowing zero-ing in
    on a specific location with individual key presses.

  * The right-meta key in combination with J, K, and L causes a left, middle, or right mouse button press event respectively.
    It sends a right-meta-up event before the button event, then a right-meta-down event after the button event so that you
    get the behavior of the button presses without the meta modifier.  The down-stroke of the key gives a mouse-down event
    and the up-stroke of the key gives a mouse-up event, so you can drag-and-drop with this.

Please be aware that if you manually invoke the keyboard_mouse program using the keyboard it's applying to at the command
line, you'll likely get the odd behavior of the return key on repeat even though you're not still holding it down.  Hitting
another key will end this repeat and it won't come back until keyboard_mouse exits and is run again.  This is caused by the
fact that keyboard_mouse grabs the raw input device and so other listeners on it (typically for X11) never see the return key
release event on the original keyboard device and instead see it coming from what looks like a different keyboard, so they
don't think it corresponds to the key-down event on the original keyboard.

This program doesn't read any sort of configuration file.  Instead, if you want to configure it to use different key
combinations or have modified behavior in another way, please just modify your copy of the source code.


    WHY I WROTE THIS
    ----------------

I wrote this because I couldn't find any easier way to get the functionality I was looking for.  I spent a couple of days
trying to get this functionality.  I did a lot of Google searches and reading and experimentation.  Here are the approaches
I tried or considered:

  * xmodmap: This didn't work because there doesn't seem to be a way to specify mouse movements

  * xbindkeys with xdotool: This partly worked, but some of the key combinations I wanted to use were grabbed by the window
    manager layer and never seen by xbindkeys.

  * A Salmon script reading from the keyboard device in /dev/input and generating X11 events with xdotool: This worked but
    left the keyboard events still going to X11, causing unintended effects.

  * The same Salmon script with xbindkeys to map the key combinations coming through to the X11 layer to nothing.  This
    partly worked but had performance problems and still left some key combinations being interpretted by the window manager
    layer and causing unintended consequences.

  * Programming my Kinesis Advantage 2 keyboard by itself:  This didn't work because I couldn't find a way to map keys to
    mouse movements.

  * Programming keyboard macros in combination with a script listening to the /dev/input device.  What I was hoping for here
    was to map my key combinations to some otherwise-unused combinations that wouldn't have side effects but could be used to
    trigger the script to call xdotool to generate the appropriate events.  This mostly worked, but I found I couldn't drag
    and drop because when the keyboard mapped a button to a macro the macro sequence would be sent on a key-down event and
    the keyboard would then send nothing for the key-up event.

  * Programming keyboard remaps in combination with a script listening to the /dev/input device.  Unlike with macros, when
    the Kinesis Advantage 2 is programmed to remap keys, the computer still gets both down and up events.  But this maps the
    whole key, not just a combination of keys, so I figured I'd map the keys I wanted to some unused code, then use xdotool
    to send the appropriate events for the keystrokes for combinations of that key that weren't what I wanted to capture.  It
    turns out that the codes used by the keyboard programming are USB key codes, which are totally different numbers than the
    codes used in the Linux /usr/input device packets.  I found the source for the Linux USB HID driver and found the table
    mapping between them.  I started experimenting and found that even some codes that seemed to be mapped by the driver and
    that I figured were probably unused still got filtered out -- I'm not sure where.  I didn't try all the codes that were
    mapped by the driver because I started worrying that actually some applications might actually do something with even the
    unusual codes and I might get weird effects unexpectedly down the road because these unusual codes would still be showing
    up in X11.

  * Modifying the Linux USB HID driver.  I didn't actually implement this approach.  I investigated it and I think it would
    work.  But I'd have to modify my system configuration to load my modified driver instead of the standard one and if I
    messed up in development I could crash my system and have to go through a reboot.  Also, if there were any hidden bugs
    once I deployed it for real I could end up with system crashes down the road.  This seemed an inferior option to this
    user-space KeyboardMouse program I've created instead.

  * Modifying the code in X11 that reads from the /dev/input devices: I also didn't actually implement this approach.  I
    believe it should work.  But the test-and-debug cycle would be painful and as X11 was updated over time I'd have to keep
    rebuilding and potentially updating my code to keep up with changes.  This seemed an inferior option to this
    KeyboardMouse program.

    License
    -------

I've placed this code in the public domain.  That means I've voluntarily given up my copyright claims on it.  You can use it
however you like.
