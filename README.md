## Pork ctf

This is a CTF challenge I made for fun.
Finding the vulnerability is pretty hard, but exploiting it is super easy once you understand the
vuln.

# The challenge:

There is a simple notes server that allows you to create, read and delete notes.

The server runs on a regular android application, and the notes are stored in the app file-system.

Your goal is to get the flag from the ADMIN user notes.

## Running the challenge:

1. Build the android app using Android Studio or ./gradlew assemble or download the pre-built apk
   from the releases.
2. Install the app on an android emulator or device.
3. Run the app regularly
4. Start playing around with another app in the same emulator or with the pc with adb forwarding.

## Given details:

1. The admin username is "ADMIN".
2. It can be solved.

## Common setup problems:
1. Make sure you change the cmake version in the ./app/build.gradle to the one you have installed.
2. You can run logcat to see the logs of the app, also you can run `netstat -ate` to check if the
   server is running.
3. I have tested the challenge on a Samsung Galaxy S8+ Android 9 real device. If the challenge doesn't work on
   your device please let me know.

## Contact:

Having a problem with the challenge?

Got the flag in a special way?

Wanna check if you're on the right path?

Contact me at [alon.ponch@gmail.com](mailto:alon.ponch@gmail.com).

## Credits:

Only to me :)