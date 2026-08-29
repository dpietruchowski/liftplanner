# Garmin Forerunner 970 integration — options without a watch app

Research notes on how LiftPlanner (Android) could exchange data with a Garmin Forerunner 970
**without installing a Connect IQ app on the watch**. Everything below relies on stock watch
firmware and, at most, the Garmin Connect phone app the user already has.

Researched 2026-08-29. Garmin's APIs and Health Connect coverage change; re-verify before building.

## Summary

| Option | Direction | Backend needed | Garmin approval | Fit for LiftPlanner |
| --- | --- | --- | --- | --- |
| Health Connect | watch → phone | no | no | good — post-workout summaries, recovery context |
| Connect Developer Program (Health/Activity API) | watch → cloud | yes | yes (enterprise) | overkill |
| Aggregators (Terra, ROOK, Vital, Spike) | watch → cloud | yes | no (paid) | overkill |
| BLE heart-rate broadcast | watch → phone | no | no | good — live HR during a set |
| FIT files (export / MTP) | watch → phone | no | no | impractical on Android |
| Reverse-engineered BLE (GFDI) | both | no | no (off-ToS) | fragile |
| Training API | phone → watch | yes | yes (enterprise) | the only sanctioned way to push plans |
| Unofficial `workout-service` | phone → watch | no | no (off-ToS) | works, may break anytime |
| Android notifications | phone → watch | no | no | free, display-only |

## Reading data from the watch

### Health Connect — simplest, no backend, no approval

Garmin Connect writes to Google Health Connect on Android. LiftPlanner reads those records
locally through the standard Health Connect API. The watch is unaware of any of it; the user
enables the integration once in Garmin Connect → Settings → Health Connect.

**Shared from workouts:** active and total calories, cycling pedal cadence, distance, elevation
gained, heart rate, speed, steps, swimming strokes.

**Shared from daily wellness:** steps and step distance, calories, floors climbed, heart rate,
sleep (with stages), weight, body fat.

**Never shared:** Body Battery, training load, aerobic/anaerobic training effect, intensity
minutes, running tolerance, endurance score, HRV status. Garmin's proprietary metrics stay in
Garmin Connect.

**One-way only.** Garmin writes to Health Connect and reads nothing back, so this channel cannot
be used to send anything to the watch.

#### The catch for a strength app

Health Connect has no data type for **load per set**. `ExerciseSegment` carries a repetition count
and segment types such as bench press, deadlift and squat, but there is no weight field anywhere in
the model. On top of that, Garmin most likely writes a strength workout as a single aggregate
session rather than per-set segments.

Assume Health Connect yields *"a 47-minute strength session, 320 kcal, here is the heart-rate
trace"* — not *"3×8 at 80 kg"*. Verify empirically on a real device before designing a feature
around per-set data.

#### Implementation notes

- Permissions are declared per record type (`READ_STEPS`, `READ_HEART_RATE`, `READ_EXERCISE`,
  `READ_SLEEP`, …).
- Only the last **30 days** are visible by default; deeper history needs
  `READ_HEALTH_DATA_HISTORY`.
- Reading while the app is backgrounded needs `READ_HEALTH_DATA_IN_BACKGROUND`.
- Publishing to Google Play with health permissions requires a data-use declaration — real
  release overhead, worth planning for.
- Incremental sync via changes tokens avoids re-scanning everything.

### BLE heart-rate broadcast — live data, zero integration

The Forerunner 970 can act as a standard BLE heart-rate sensor: **Heart Rate Service `0x180D`**,
**Heart Rate Measurement characteristic `0x2A37`** via notifications, roughly one sample per
second. Battery Service `0x180F` is usually exposed alongside it.

The measurement frame carries beats per minute, a sensor-contact flag and optionally energy
expended. It also has a slot for **R-R intervals, which Garmin does not populate** — no HRV from
this channel.

That is the whole payload: live heart rate, nothing else. In exchange it is instant, works
offline and does not involve Garmin Connect at all.

**Practical constraints**

- The user enables it manually: *Watch Settings → Health & Wellness → Wrist Heart Rate →
  Broadcast Heart Rate*. The *Broadcast During Activity* option turns it on automatically when an
  activity starts, which suits a workout flow better.
- It noticeably shortens battery life.
- Android needs `BLUETOOTH_SCAN` and `BLUETOOTH_CONNECT` (plus location on API ≤ 30).
- `QLowEnergyController` from Qt Bluetooth handles this with no extra dependency — it is a plain
  standard GATT profile.

### Garmin Connect Developer Program (Health / Activity API)

Cloud-to-cloud. OAuth 2.0, and Garmin pushes data to a registered webhook when the watch syncs.
Rich payload: 100+ activity types, GPS, HRV, VO2max. No licensing fees, applications reviewed in
about two business days, but the program is formally *"available for enterprise use"* and hobby
projects are frequently rejected. Requires a server with a public URL.

### Aggregators

Terra, ROOK, Vital, Spike and Junction resell Garmin access: the user authenticates through the
aggregator's widget, and normalized data lands on your webhook. Sidesteps Garmin's approval, but
is paid, still needs a backend, and several of them require registering your own Garmin developer
credentials anyway.

### FIT files

Export from Garmin Connect, or mount the watch as an MTP device over USB. Fine on a desktop,
impractical on Android over OTG.

### Reverse-engineered BLE

Gadgetbridge decoded Garmin's GFDI protocol and talks to the watch directly over BLE
(`garmin-ble`, `garmin-bridge` build on that work). Unofficial `connect.garmin.com` endpoints are
similarly documented (`python-garminconnect`). Functional, outside Garmin's terms, and liable to
break without notice.

## Writing data to the watch

### Training API — the sanctioned path

Publishes workouts and training plans to the Garmin Connect calendar; the user syncs and the
watch guides them step by step. It covers strength training: exercises, sets, repetitions,
weights and rest periods. Same enterprise application and backend requirement as the other
Developer Program APIs.

### Unofficial workout service

`POST https://connect.garmin.com/workout-service/workout` with a bearer token, against 1000+
official Garmin exercises with sets, reps and weights. Community documentation exists
(`n1t3k/garmin-strength-api`, `mkuthan/garmin-workouts`). No approval needed, but reverse
engineered.

### Android notifications

A plain `Notification` from LiftPlanner is mirrored to the watch by Garmin Connect. Zero work,
but display-only — no interaction comes back.

### Connect IQ Mobile SDK — ruled out

Requires a Monkey C app on the watch, and on Android it routes through Garmin Connect Mobile as a
broker anyway. Out of scope by definition.

## What is impossible without a watch app

- Interactive wrist UI — ticking off sets, driving the rest timer from the watch.
- Two-way real-time communication.
- Receiving events from the watch mid-workout.

Without Connect IQ the watch is either a data source (after the fact, or live HR broadcast) or a
plan recipient. Never both at once, never interactive.

## Recommendation for LiftPlanner

The two no-approval options complement rather than overlap:

- **BLE heart-rate broadcast** for live heart rate during a set.
- **Health Connect** for the post-workout summary plus recovery context (sleep, resting heart
  rate, weight).

Neither requires a backend, a Garmin application or anything installed on the watch. The training
plan itself — exercises, sets, loads — stays in LiftPlanner, because neither channel can carry it.
Pushing plans to the watch means the Training API or the unofficial workout service.

## Sources

- [Garmin clarifies what data it's sharing with Google Health Connect](https://garminrumors.com/garmin-clarifies-what-data-its-sharing-with-google-health-connect-heres-the-full-list/)
- [Here's everything Garmin will and won't share with Google Health Connect — Android Central](https://www.androidcentral.com/wearables/garmin/heres-everything-garmin-will-and-wont-share-with-google-health-connect)
- [Sharing Your Garmin Connect Data With Health Connect — Garmin Support](https://support.garmin.com/en-US/?faq=JToBEy0jfe6pIygark2Ui5)
- [Forerunner 970 Owner's Manual — Broadcasting Heart Rate Data](https://www8.garmin.com/manuals/webhelp/GUID-025D75CF-3445-49E1-8D81-1AA74AB4E00F/EN-US/GUID-D8D363C2-0690-48D4-95E2-A3557E7D53C2.html)
- [Can Garmin Wearables Broadcast Heart Rate Data? — Garmin Support](https://support.garmin.com/en-US/?faq=Zj1947s6pqAHzBCAhLhrC9)
- [Broadcast Heart Rate R-R intervals? — Garmin Forums](https://forums.garmin.com/sports-fitness/running-multisport/f/forerunner-745/243411/broadcast-heart-rate-r-r-intervals)
- [Garmin Connect Developer Program](https://developer.garmin.com/gc-developer-program/)
- [Program FAQ — Garmin Developers](https://developer.garmin.com/gc-developer-program/program-faq/)
- [Training API — Garmin Developers](https://developer.garmin.com/gc-developer-program/training-api/)
- [Mobile SDK for Android — Connect IQ](https://developer.garmin.com/connect-iq/core-topics/mobile-sdk-for-android/)
- [Garmin Protocol — Gadgetbridge](https://gadgetbridge.org/internals/specifics/garmin-protocol/)
- [n1t3k/garmin-strength-api](https://github.com/n1t3k/garmin-strength-api)
- [Garmin API Integration — Terra](https://tryterra.co/integrations/garmin)
- [Garmin Integration — ROOK](https://www.tryrook.io/wearable-api-sdk/integrations/garmin)
