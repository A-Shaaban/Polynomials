QB64 COMPILER V0.82
===================

V0.82 Specific
==============
Primarily implements:
1) PRINT USING [#] command
2) STATIC command (all usages)
3) Major bug fixes to the QB64 IDE
4) Major bug fixes and improvements to the QB64 compiler
This is a Windows only release of V0.82, a Linux version will become available in 1-2 weeks time.

V0.81x Specific
===============
V0.81x implements 2 major changes:
1) Provides an IDE remarkably similar (in look and interface) to the QBASIC IDE called qb64.exe
notes:
-qb64.exe can perform command line compilation (similar to previous versions of QB64) using the -c switch, for example:
 qb64 -c mycode.bas
-many bugs/issues remain to be fixed, report them in the forum (gripes are ok too!)
-many more features are planned, at present the IDE is essentially a text editor that checks your code as you type it
-code may have "OK" status but fail C++ compilation (on F5) due to the underlying QB64 compiler not checking code thouroughly enough
-programs using the underscore (_) symbol for line don't work in the IDE yet
-the IDE features an autosave feature which silently saves your code after every 10 changes, you will be automatically prompted to restore this upon restart if you didn't exit normally (via the file menu's "exit" command)
-C++ compilation failure and some preprocessing errors are reported with incorrect line numbers, which can be misleading

2) Linux support
notes:
-you must have downloaded the Linux version of QB64 from the qb64.net forums and followed necessary installation steps
-64 bit Linux systems are not supported yet (it may be possible to edit ./internal/c/makeline.txt to force compilation for a 32-bit system)
-EOF() doesn't work correctly
-Type _UNSIGNED INTEGER64 (only the unsigned version) cannot be used due to an incompatibility
-check forward slashes and backslashes are correct for the Linux filesystem (QB64 will address this automatically in later versions)
-check case of referenced filenames carefully (QB64 will address this automatically in later versions)
-unhandled (ie. not handled by ON ERROR GOTO ...) errors when your code is running currently result in immediate program termination without warning
-lots of other compatibility problems exist and will be fixed in later versions, report them in the forum

general notes about V0.81x:
-compile.exe is not in V0.81x but will be provided alongside qb64.exe in subsequent versions
-temporarily removed support for SETTING the time/date

V0.81 Specific
==============
V0.81 primarily implements support for constants (refer to the CONST keyword)
Handling of numbers written in the code has been improved to support all type symbol extensions (even the newer QB64 types), this also applies to &H..., &O... and the newly implemented binary prefix &B... (note that &B... only currently works when used with numbers directly in your code, further support for &B... will be added later). This provides better compatibility with QBASIC's handling of numbers.
Several bugs causing compilation to fail on valid code have been corrected, but always check that the program to be compiled doesn't include any as yet unimplemented features listed later in this document.
No new sample programs have been added in this release.

V0.8 Specific
=============
V0.8 primarily implements the QB64 2D prototype 1.0, a fully integrated 2D graphics interface which encapsulates and extends upon QBASIC's standard graphics capabilities. It incorporates .TGA .BMP .PNM .XPM .XCF .PCX .GIF .JPG .TIF .LBM .PNG graphics formats, .TTF (true type) fonts, 32-bit RGBA, alpha blended drawing, color-key'd palette indexed images, RGB to palette matching, stretching, flipping, mirroring, and other functionality. Multiple graphics surfaces can be used of differing dimensions and formats simultaneously, and operations can be performed any these surfaces interchangeably.

The 2D prototype is fully implemented but still in the process of being documented in detail. Please refer to the following webpage for the latest documentation:
http://www.qb64.net/2d
Feel free to enquire about the 2D prototype & its usage at:
http://www.network54.com/Forum/585676/
No official example programs are available which demonstrate the 2D prototype yet, however they will be released soon.

Thanks for trying QB64!
The goal of QB64 is to create programs from BASIC source for Windows, Mac OSX and Linux whilst being 100% compatible with Microsoft QBASIC/QB4.5

Make sure original .BAS files from QB4.5 were saved in text format not its compressed file format.

Check "SAMPLES.TXT" for information about included .BAS example programs.

UNIMPLEMENTED FEATURES
======================
The majority of QBASIC commands are implemented.
Only the following are NOT IMPLEMENTED yet:
 -ON ... GOTO/GOSUB EVENTS (however, ON ERROR GOTO is implemented)
 -$INCLUDE metacommand
 -Devices (COM...,SCRN:,LPT...,KYBD:,CONS:) in an OPEN statement
 -CALL INTERRUPT
 -CALL ABSOLUTE (has limited support including only PUSH/POP/MOV/INT 33h)
 -Port access (has limited support including OUT &H3C8/&H3C9 and INP &H3DA/&H60)
 -Multimodular support (COMMON, etc.)
 -Other commands: TRON/TROFF, CHAIN, RUN, FILES, FILEATTR, ENVIRON, ENVIRON$, LPRINT, LPOS, DRAW, CLEAR, FIELD, LOCK, UNLOCK, IOCTL, IOCTL$, PEN, STICK, STRIG, SETMEM, FRE, FIELD, KEY ON/OFF

QB64 SPECIFIC FEATURES (THAT QBASIC DOESN'T HAVE)
=================================================

INPUT Protection
----------------
"Intelligently" restricts keyboard input. Avoids "redo from start" messages. Limits screen space used by input for fixed length strings and numbers (eg. INTEGER cannot use more than 6 spaces)

New Data Types
--------------
_BIT			name` or name`1
_UNSIGNED _BIT		name~` or name~`1
_BIT*4			name`4
_UNSIGNED _BIT*4	name~`4
_BYTE			name%%
_UNSIGNED _BYTE		name~%%
INTEGER			name%
_UNSIGNED INTEGER	name~%
LONG			name&
_UNSIGNED LONG		name~&
_INTEGER64		name&&
_UNSIGNED _INTEGER64	name~&&
SINGLE			name!
DOUBLE			name#
_FLOAT			name##
STRING			name$
STRING*100		name$100

_DEFINE Command
---------------
Instead of having DEFBIT,DEFUBT,DEFBYT,DEFUBY etc. for all the new data types there is a simple command which can be used like DEFINT for new types (or old types if you want to) as follows:
_DEFINE A-C,Z AS DOUBLE 'the same as DEFDBL A-C,Z

Larger Maximum RANDOM File Access Field Sizes
---------------------------------------------
For RANDOM access files, record lengths can now be greater than 32767 bytes. Variable length string headers allow for larger strings whilst still being 100% QBASIC compatible with smaller strings.

BLOAD/BSAVE Limit
-----------------
Can save/load 65536 bytes, not just 65535.

_MK$(variable-type,value)
-------------------------
Like MKI$/MKS$/etc., this converts numbers into a binary string. The advantage of _MK$() is it allows conversion into the newer QB64 data types as apose to just the QBASIC types. Example usage:
a$=_MK$(_UNSIGNED _INTEGER64,100)
Note: _CV(variable-type,string) can also be used.

_ROUND 
------
This can be used to round values to integers (CINT & CLNG imposed limitations on the output)

Graphics GET/PUT
----------------
GET supports a new optional argument. If used the area to store can be partially/all off-screen and off-screen pixels are set to the value specified:
GET (-10,-10)-(10,10),a,3
PUT format has been extended to PUT[{STEP}](?,?),?[,[{_CLIP}][{PSET|PRESET|AND|OR|XOR}][,?]] where _CLIP allows drawing partially/all off-screen and the final optional argument can specify a mask color to skip when drawing.

Better Sound/Audio Support
--------------------------
Support for playing .MID, .WAV, .MP3 and many other formats.
Commands include:
_SNDPLAYFILE		Simple command to play a sound file (with limited options)
_SNDOPEN		Returns a handle to a sound file
_SNDCLOSE		Unloads a sound file (waits until after it has finished playing)
_SNDPLAY		Plays a sound
_SNDSTOP		Stops a playing (or paused) sound
_SNDPLAYING		Returns whether a sound is being played
_SNDLOOP		Like _SNDPLAY but sound is looped
_SNDLIMIT		Stops playing a sound after it has been playing for a set number of seconds
_SNDGETPOS		Returns to current playing position in seconds
_SNDCOPY		Copies a sound (so two or more of the same sound can be played at once)
_SNDPLAYCOPY		Copies a sound, plays it and automatically closes the copy
_SNDPAUSE		Pauses a sound
_SNDPAUSED		Checks if a sound is paused
_SNDLEN			Returns the length of a sound in seconds
_SNDVOL			Sets the volume of a sound
_SNDBAL			Sets the balance/3D position of a sound
_SNDSETPOS		Changes the current/starting playing position of a sound in seconds
For more information, read "AUDIO.TXT" in your "QB64" folder.
Also check out "AUDIO.BAS" in the samples folder, you'll need some audio files to test this with!

Mouse Support
-------------
_MOUSESHOW		Displays the mouse cursor (sub)
_MOUSEHIDE		Hides the mouse cursor (sub)
_MOUSEINPUT		MUST BE CALLED UNTIL IT RETURNS 0, it reads the next mouse message and returns -1 if new information was available otherwise 0 (func)
_MOUSEX			(func)
_MOUSEY			(func)
_MOUSEBUTTON(n)		-1 if button n (a value of 1 or more) is down