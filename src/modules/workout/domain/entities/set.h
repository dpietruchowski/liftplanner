#pragma once

class Set final
{
public:
    Set();
    Set(int repetitions, int weight);

    int id() const;
    int exerciseId() const;
    int repetitions() const;
    int weight() const;
    bool completed() const;

    void setId(int id);
    void setExerciseId(int exerciseId);
    void setRepetitions(int repetitions);
    void setWeight(int weight);
    void setCompleted(bool completed);

private:
    void validate() const;

    int m_id{-1};
    int m_exerciseId{-1};
    int m_repetitions{0};
    int m_weight{0};
    bool m_completed{false};
};
