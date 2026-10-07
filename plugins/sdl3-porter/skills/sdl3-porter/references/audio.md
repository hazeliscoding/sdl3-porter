# Audio traps

Read this when the project uses SDL audio. Each trap compiles cleanly against SDL3 with warnings as errors, and breaks at runtime.

## audio-stream-paused

**What compiles:**

```c
SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
SDL_PutAudioStreamData(stream, samples, size);    /* never plays */
```

**What breaks:** `SDL_OpenAudioDeviceStream` opens its device paused. Nothing plays, and no callback fires, until the app resumes the device.

The SDL2 code this replaces called `SDL_PauseAudioDevice(dev, 0)` to start playback. In SDL3 that call no longer compiles, and the migration guide says devices opened with `SDL_OpenAudioDevice` no longer start paused, so the unpause looks safe to delete. That's true only for `SDL_OpenAudioDevice`, not for `SDL_OpenAudioDeviceStream`.

**How to find it:** search the ported sources for streams opened this way, and for the calls that resume or pause them:

```
SDL_OpenAudioDeviceStream
SDL_Resume(AudioStreamDevice|AudioDevice)|SDL_PauseAudio(StreamDevice|Device)
```

Every stream from `SDL_OpenAudioDeviceStream` needs its device resumed, with `SDL_ResumeAudioStreamDevice(stream)` or `SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(stream))`, before the app expects sound. `SDL_PauseAudioDevice` takes one argument in SDL3 and always pauses, so a `SDL_PauseAudioDevice(dev, 0)` trimmed to `SDL_PauseAudioDevice(dev)` pauses instead of resuming. Streams bound with `SDL_BindAudioStream` to a device from `SDL_OpenAudioDevice` need no resume.

**Fix:** resume the device once the stream is open:

```c
SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
if (!stream) {
    SDL_Log("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
    return 1;
}
SDL_ResumeAudioStreamDevice(stream);
```

**Source:** SDL `docs/README-migration.md`, `SDL_audio.h` section: "Audio devices, opened by SDL_OpenAudioDevice(), no longer start in a paused state", with the SDL2 migration examples that resume the stream's device. `SDL_OpenAudioDeviceStream` in `SDL_audio.h` ([wiki](https://wiki.libsdl.org/SDL3/SDL_OpenAudioDeviceStream)): "the audio device begins paused. [...] The audio device should be resumed with SDL_ResumeAudioStreamDevice()."

**Fixture:** `fixtures/audio-stream-paused/`.

## mix-volume-float

**What compiles:**

```c
#define SDL_MIX_MAXVOLUME 128    /* re-added because SDL3 removed it */
SDL_MixAudio(dst, src, SDL_AUDIO_S16, len, SDL_MIX_MAXVOLUME / 2);
SDL_MixAudio(dst, src, SDL_AUDIO_S16, len, 128);
```

**What breaks:** SDL2's `SDL_MixAudioFormat` took an integer volume from 0 to `SDL_MIX_MAXVOLUME` (128). SDL3 renamed it `SDL_MixAudio`, and its volume is a float from 0.0 to 1.0. C converts the integer without a word, so SDL2's half volume, 64, becomes 64 times full volume. SDL3 doesn't clamp the volume. 8- and 16-bit samples wrap around into loud noise: a 16-bit sample of 8000 mixed at 64 comes out as -12288. 32-bit and float samples clip at full scale.

The rename scripts rename the function and leave the volume alone. Code that still uses `SDL_MIX_MAXVOLUME` doesn't compile, because SDL3 removed the macro. Re-defining it as 128, or replacing it with a number, makes the call compile and keeps the wrong scale. SDL2's four-argument `SDL_MixAudio`, which mixed in the format of the device from `SDL_OpenAudio`, doesn't compile against SDL3's five-argument one either. When you add the format, convert the volume too.

**How to find it:** search the ported sources for every mixing call, and for SDL2's volume scale:

```
SDL_MixAudio
SDL_MIX_MAXVOLUME|MIX_?MAX_?VOLUME
```

Calls often span several lines, so read each whole call. Trace its last argument back to where it's set, and compare it with the SDL2 original in `git diff`. A volume that came from SDL2 is on the 0–128 scale, and only 0 means the same on both scales. Convert everything else: literals, including 1, which was 1/128 of full volume; anything computed on the 0–128 scale; and integer variables. Don't count on the compiler: MSVC `/W4` warns (C4244) about non-constant `int` expressions, but GCC and Clang don't with `-Wall -Wextra`, and no compiler warns about literals, macros or a plain 8- or 16-bit integer variable. A 0–128 constant is fine as the divisor of a float division, as in `volume / 128.0f`, but never as the volume itself. With integers on both sides, `volume / 128` is 0 for every volume below 128, and SDL3 mixes nothing.

**Fix:** pass a float from 0.0 to 1.0:

```c
SDL_MixAudio(dst, src, SDL_AUDIO_S16, len, 0.5f);
```

`SDL_MIX_MAXVOLUME` becomes `1.0f`. A volume the project keeps on SDL2's 0–128 scale converts with `volume / 128.0f`. Storing it as a float from 0 to 1 is better.

**Source:** SDL `docs/README-migration.md`, `SDL_audio.h` section: "SDL_MixAudioFormat() and SDL_MIX_MAXVOLUME have been removed in favour of SDL_MixAudio(), which now takes the audio format, and a float volume between 0.0 and 1.0." `SDL_MixAudio` in `SDL_audio.h` ([wiki](https://wiki.libsdl.org/SDL3/SDL_MixAudio)): "volume ranges from 0.0 - 1.0, and should be set to 1.0 for full audio volume." The wrap comes from SDL's `src/audio/SDL_mixer.c` at `release-3.4.18`, not from the docs: it rounds `volume * 128` to an `int` without clamping it, and scales 8- and 16-bit samples within their own type before the overflow clipping the header describes. 32-bit and float samples don't overflow when scaled, so they clip.

**Fixture:** `fixtures/mix-volume-float/`.

## audio-init-implicit

**What compiles:**

```c
SDL_Init(SDL_INIT_VIDEO);    /* SDL2's SDL_OpenAudio started audio by itself */
/* ... */
SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, callback, NULL);    /* NULL */
```

**What breaks:** SDL2's legacy `SDL_OpenAudio` initialized the audio subsystem if the program hadn't, so many programs initialized only video and still had sound. SDL3 has no `SDL_OpenAudio`, and its replacements, `SDL_OpenAudioDeviceStream` and `SDL_OpenAudioDevice`, fail with "Audio subsystem is not initialized" unless something initialized `SDL_INIT_AUDIO` first. The port runs silent, or crashes if it uses the stream without checking it. SDL2's `SDL_OpenAudioDevice` never initialized audio by itself, so code that called it already initializes audio.

**How to find it:** check `git diff` for `SDL_OpenAudio(` in the SDL2 code, then search the ported sources for how SDL starts and where audio opens:

```
SDL_Init(SubSystem)?\s*\(|SDL_INIT_AUDIO
SDL_OpenAudioDevice\w*\s*\(
```

If the project opens an audio device and no `SDL_Init` or `SDL_InitSubSystem` call passes `SDL_INIT_AUDIO` before it, add it. Watch for SDL2's `SDL_INIT_EVERYTHING` too: SDL3 removed it, and the flags that replace it have to include `SDL_INIT_AUDIO` when the program plays sound.

**Fix:** initialize audio explicitly:

```c
if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
}
```

**Source:** SDL `docs/README-migration.md`, `SDL_audio.h` section: "SDL3 will not implicitly initialize the audio subsystem on your behalf if you open a device without doing so. Please explicitly call SDL_Init(SDL_INIT_AUDIO) at some point." SDL2's implicit initialization is in `SDL_OpenAudio`, in `src/audio/SDL_audio.c` at `release-2.32.10`.

**Fixture:** `fixtures/audio-init-implicit/`.
