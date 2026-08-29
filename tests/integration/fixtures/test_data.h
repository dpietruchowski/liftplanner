#pragma once

#include <QString>

namespace TestData
{

inline const QString THREE_WORKOUTS_JSON = R"({
    "user_profile": null,
    "workouts": [
        {
            "name": "Push Day",
            "exercises": [
                {
                    "name": "Bench Press",
                    "rest_seconds": 120,
                    "sets": [
                        {"repetitions": 10, "weight": 60},
                        {"repetitions": 8, "weight": 70},
                        {"repetitions": 6, "weight": 80}
                    ]
                },
                {
                    "name": "Overhead Press",
                    "rest_seconds": 90,
                    "sets": [
                        {"repetitions": 10, "weight": 30},
                        {"repetitions": 8, "weight": 35}
                    ]
                }
            ]
        },
        {
            "name": "Pull Day",
            "exercises": [
                {
                    "name": "Barbell Row",
                    "rest_seconds": 120,
                    "sets": [
                        {"repetitions": 10, "weight": 50},
                        {"repetitions": 8, "weight": 60}
                    ]
                },
                {
                    "name": "Pull-ups",
                    "rest_seconds": 90,
                    "sets": [
                        {"repetitions": 8, "weight": 0},
                        {"repetitions": 6, "weight": 0}
                    ]
                }
            ]
        },
        {
            "name": "Leg Day",
            "exercises": [
                {
                    "name": "Squat",
                    "rest_seconds": 180,
                    "sets": [
                        {"repetitions": 8, "weight": 80},
                        {"repetitions": 6, "weight": 90},
                        {"repetitions": 4, "weight": 100}
                    ]
                }
            ]
        }
    ]
})";

inline const QString SINGLE_WORKOUT_JSON = R"({
    "user_profile": null,
    "workouts": [
        {
            "name": "Full Body",
            "exercises": [
                {
                    "name": "Squat",
                    "rest_seconds": 120,
                    "sets": [
                        {"repetitions": 5, "weight": 100}
                    ]
                },
                {
                    "name": "Bench Press",
                    "rest_seconds": 120,
                    "sets": [
                        {"repetitions": 5, "weight": 80}
                    ]
                }
            ]
        }
    ]
})";

inline const QString EMPTY_WORKOUT_JSON = R"({
    "user_profile": null,
    "workouts": [
        {
            "name": "Rest Day",
            "exercises": []
        }
    ]
})";

inline const QString MIXED_WORKOUT_JSON = R"JSON({
    "user_profile": null,
    "workouts": [
        {
            "name": "Hybrid Day",
            "exercises": [
                {
                    "name": "Bench Press",
                    "kind": "strength",
                    "rest_seconds": 120,
                    "sets": "10x60kg,8x70kg,6x80kg"
                },
                {
                    "name": "Pull-ups",
                    "kind": "bodyweight",
                    "rest_seconds": 90,
                    "sets": "8xBW,6xBW,5xBW-10kg"
                },
                {
                    "name": "Plank",
                    "kind": "mobility",
                    "rest_seconds": 60,
                    "sets": "45s,45s,3min"
                },
                {
                    "name": "Burpee intervals",
                    "kind": "interval",
                    "rest_seconds": 60,
                    "sets": "8x(20s/10s)"
                },
                {
                    "name": "Run",
                    "kind": "cardio",
                    "rest_seconds": 0,
                    "sets": "5km@24min"
                }
            ]
        }
    ]
})JSON";

inline const QString BROKEN_SETS_JSON = R"JSON({
    "user_profile": null,
    "workouts": [
        {
            "name": "Broken Day",
            "exercises": [
                {
                    "name": "Bench Press",
                    "rest_seconds": 120,
                    "sets": "10x60kg,gibberish,6x80kg"
                }
            ]
        }
    ]
})JSON";

}  // namespace TestData
