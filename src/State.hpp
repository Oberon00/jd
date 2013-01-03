#ifndef STATE_HPP_INCLUDED
#define STATE_HPP_INCLUDED STATE_HPP_INCLUDED

#include <boost/noncopyable.hpp>


/* abstract */ class State: private boost::noncopyable {
public:
    virtual ~State() { }

    // used for lazy initialization, so the state is only loaded, when needed
    // called when the state is pushed
    virtual void initialize() { }
        
    // Called while the previous state is still active (i.e. on top of the stack)
    // can be used to get information from the previous state. Always called before resume.
    virtual void prepare() { }

    // called when the state is resuming after being paued and also on normal startup
    virtual void resume() = 0;
        
    // called when the state is paused and also always before stop()
    virtual void pause() = 0;

    // called before the state is deactivated.
    virtual void stop() { }

    // called after the state is popped
    virtual void cleanup() { }
};

#endif
