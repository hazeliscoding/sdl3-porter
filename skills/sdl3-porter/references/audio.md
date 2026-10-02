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
SDL_OpenAudioDeviceStream\(
SDL_Resume(AudioStreamDevice|AudioDevice)\(|SDL_PauseAudio(StreamDevice|Device)\(
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
