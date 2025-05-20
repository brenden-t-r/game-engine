/*
 *      ___           ___           ___           ___
 *     /  /\         /  /\         /__/\         /  /\
 *    /  /:/_       /  /::\       |  |::\       /  /:/_
 *   /  /:/ /\     /  /:/\:\      |  |:|:\     /  /:/ /\
 *  /  /:/_/::\   /  /:/~/::\   __|__|:|\:\   /  /:/ /:/_
 * /__/:/__\/\:\ /__/:/ /:/\:\ /__/::::| \:\ /__/:/ /:/ /\
 * \  \:\ /~~/:/ \  \:\/:/__\/ \  \:\~~\__\/ \  \:\/:/ /:/
 *  \  \:\  /:/   \  \::/       \  \:\        \  \::/ /:/
 *   \  \:\/:/     \  \:\        \  \:\        \  \:\/:/
 *    \  \::/       \  \:\        \  \:\        \  \::/
 *     \__\/         \__\/         \__\/         \__\/
 *
 */

#include "../engine/platform/platform.h"
#include "../engine/engine/game.h"
#include "../engine/samples/pongGame.h"
#include "../engine/entry.h"
#include "cstdio"

// Base coroutine class
class Coroutine {
public:
    enum class State { Idle, Running, Completed };

private:
    State state = State::Idle;

public:
    virtual ~Coroutine() {}

    void Start() {
        if (state == State::Idle) {
            state = State::Running;
            OnStart();
        }
    }

    bool Update(float deltaTime) {
        if (state != State::Running) return false;

        bool isComplete = OnUpdate(deltaTime);
        if (isComplete) {
            state = State::Completed;
            OnComplete();
        }

        return isComplete;
    }

    void Reset() {
        state = State::Idle;
        OnReset();
    }

    bool IsCompleted() const { return state == State::Completed; }
    State GetState() const { return state; }

    // Virtual methods to be overridden
    virtual void OnStart() {}
    virtual bool OnUpdate(float deltaTime) = 0;
    virtual void OnComplete() {}
    virtual void OnReset() {}

    // Clone method for copying coroutines
    virtual Coroutine* Clone() const { return nullptr;}
};

// Wait for specified seconds
class WaitXSeconds : public Coroutine {
private:
    float duration;
    float elapsed = 0;

public:
    WaitXSeconds(float seconds) : duration(seconds) {}

    bool OnUpdate(float deltaTime) override {
        elapsed += deltaTime;
        printf("Waiting: %.2f / %.2f seconds\n", elapsed, duration);
        return elapsed >= duration;
    }

    void OnReset() override {
        elapsed = 0;
    }

    Coroutine* Clone() const override {
        return new WaitXSeconds(duration);
    }
};

class MyCoroutine : public Coroutine {
public:
    int states = 3;
    int state = 0;

    bool OnUpdate(float deltaTime) override {
        switch (state) {
            case 0:
                printf("Hi\n");
                state += 1;
                return false;
            case 1:
                return true;
            default:
                return false;
        }
    }
};

// Coroutine container for sequential execution
class SequentialCoroutines : public Coroutine {
private:
    std::vector<Coroutine*> routines;
    int currentIndex = 0;
    bool ownRoutines;

public:
    SequentialCoroutines(bool ownsRoutines = true) : ownRoutines(ownsRoutines) {}

    ~SequentialCoroutines() {
        if (ownRoutines) {
            for (auto r : routines) {
                delete r;
            }
        }
    }

    void Add(Coroutine* routine) {
        routines.push_back(routine);
    }

    void OnStart() override {
        currentIndex = 0;

        if (!routines.empty()) {
            routines[0]->Start();
        }
    }

    bool OnUpdate(float deltaTime) override {
        if (routines.empty() || currentIndex >= routines.size()) {
            return true;  // Complete if no routines or all finished
        }

        // Update current routine
        if (routines[currentIndex]->Update(deltaTime)) {
            // Current routine is done, move to next
            currentIndex++;

            if (currentIndex < routines.size()) {
                routines[currentIndex]->Start();
            } else {
                return true;  // All complete
            }
        }

        return false;  // Still running
    }

    void OnReset() override {
        currentIndex = 0;
        for (auto r : routines) {
            r->Reset();
        }
    }

    Coroutine* Clone() const override {
        SequentialCoroutines* clone = new SequentialCoroutines(true);
        for (auto r : routines) {
            clone->Add(r->Clone());
        }
        return clone;
    }
};

// Coroutine container for parallel execution
class ParallelCoroutines : public Coroutine {
private:
    std::vector<Coroutine*> routines;
    bool ownRoutines;

public:
    ParallelCoroutines(bool ownsRoutines = true) : ownRoutines(ownsRoutines) {}

    ~ParallelCoroutines() {
        if (ownRoutines) {
            for (auto r : routines) {
                delete r;
            }
        }
    }

    void Add(Coroutine* routine) {
        routines.push_back(routine);
    }

    void OnStart() override {
        for (auto r : routines) {
            r->Start();
        }
    }

    bool OnUpdate(float deltaTime) override {
        if (routines.empty()) {
            return true;  // Complete if no routines
        }

        bool allComplete = true;

        // Update all routines
        for (auto r : routines) {
            if (!r->IsCompleted()) {
                r->Update(deltaTime);

                if (!r->IsCompleted()) {
                    allComplete = false;
                }
            }
        }

        return allComplete;
    }

    void OnReset() override {
        for (auto r : routines) {
            r->Reset();
        }
    }

    Coroutine* Clone() const override {
        ParallelCoroutines* clone = new ParallelCoroutines(true);
        for (auto r : routines) {
            clone->Add(r->Clone());
        }
        return clone;
    }
};

SequentialCoroutines* getSequence(Coroutine** routines, int count) {
    auto sequence = new SequentialCoroutines(false);
    for (int i = 0; i < count; i++) {
        sequence->Add(routines[i]);
    }
    return sequence;
}

class SampleGame : public Game {
public:
    using Game::Game;
    void Start() override {
        platform->LoadShaders();

//        sequence = new SequentialCoroutines(false);
//        sequence->Add(new WaitXSeconds(2.0f));
//        sequence->Add(new MyCoroutine());
//        sequence->Add(new WaitXSeconds(3.0f));
//        sequence->Start();
        Coroutine* routines1[3] = {
                new WaitXSeconds(2.0f),
                new WaitXSeconds(1.0f),
        };
        Coroutine* routines2[3] = {
                new WaitXSeconds(2.0f),
                new WaitXSeconds(1.0f),
                getSequence(routines1, 3)
        };
        sequence = getSequence(routines2, 1);

        parallel = new ParallelCoroutines(false);
        parallel->Add(new WaitXSeconds(2.0f));
        parallel->Add(new MyCoroutine());
        parallel->Add(new WaitXSeconds(3.0f));
        parallel->Start();


        Coroutine* routines3[3] = {
                sequence, parallel
        };
    }

    float deltaTime = 0;
    void Update() override {
        parallel->Update(deltaTime);
        deltaTime += 0.1;
    }
private:
    SequentialCoroutines* sequence;
    ParallelCoroutines* parallel;
};

int RealMain(Platform* platform) {
    platform->Init();
    Game* game = new SampleGame(platform);
    game->Start();
    RunLoop(platform, game);
    platform->Shutdown();
    delete game;
    delete platform;
    return 0;
}
