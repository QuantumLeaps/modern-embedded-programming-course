#ifndef _H_STATEMAP
#define _H_STATEMAP

//
// The contents of this file are subject to the Mozilla Public
// License Version 1.1 (the "License"); you may not use this file
// except in compliance with the License. You may obtain a copy
// of the License at http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an
// "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
// implied. See the License for the specific language governing
// rights and limitations under the License.
//
// The Original Code is State Machine Compiler (SMC).
//
// The Initial Developer of the Original Code is Charles W. Rapp.
// Portions created by Charles W. Rapp are
// Copyright (C) 2000 - 2007. Charles W. Rapp.
// All Rights Reserved.
//
// Contributor(s):
//
// Namespace
//    statemap
//
// Description
//  This namespace contains the finite state machine context
//  class. The user can derive FSM contexts from this class and
//  interface to them with the methods of this class.
//
// Notes
//  The finite state machine needs to be initialized to the
//  starting state of the FSM.  This must be done manually in
//  the constructor of the derived class.
//
// Author
//    C. W. Rapp
//
// RCS ID
// Id: statemap.h,v 1.19 2014/09/06 19:31:28 fperrad Exp
//
// CHANGE LOG
// (See bottom of file)
//

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#if defined(SMC_USES_IOSTREAMS)
#include <iostream>
#endif // SMC_USES_IOSTREAMS
#if defined(SMC_NO_EXCEPTIONS)
#include <cassert>
#endif // SMC_NO_EXCEPTIONS
#include <cstdio>
#elif defined(WIN32)
#if defined(SMC_USES_IOSTREAMS)
#include <iostream>
#endif // SMC_USES_IOSTREAMS
#if defined(SMC_NO_EXCEPTIONS)
#include <cassert>
#endif // SMC_NO_EXCEPTIONS
#include <windows.h>
#else
#if defined(SMC_USES_IOSTREAMS)
#include <iostream.h>
#endif // SMC_USES_IOSTREAMS
#if defined(SMC_NO_EXCEPTIONS)
#include <assert.h>
#endif // SMC_NO_EXCEPTIONS
#include <stdio.h>
#endif
#if ! defined(SMC_NO_EXCEPTIONS)
#include <stdexcept>
#include <cstring>
#endif

// Limit names to 100 ASCII characters.
// Why 100? Because it is a round number.
#define MAX_NAME_LEN 100

namespace statemap
{
//---------------------------------------------------------------
// Routines.
//

#ifdef SMC_FIXED_STACK
    // When static memory is used, a string has only one copy.
    inline char* copyString(const char *s)
    {
        // Cast your const upon the waters and see what blows up.
        return (const_cast<char *>(s));
    }
#else // ! SMC_FIXED_STACK
    inline char* copyString(const char *s)
    {
        char *retval = NULL;

        if (s != NULL)
        {
            retval = new char[MAX_NAME_LEN + 1];
            retval[MAX_NAME_LEN] = '\0';
            (void) std::strncpy(retval, s, MAX_NAME_LEN);
        }

        return (retval);
    }
#endif // ! SMC_FIXED_STACK

//---------------------------------------------------------------
// Exception Classes.
//

#ifndef SMC_NO_EXCEPTIONS
    // Base class for all SMC exceptions.
    class SmcException :
        public std::runtime_error
    {
    //-----------------------------------------------------------
    // Member methods
    //
    public:

        // Destructor.
        virtual ~SmcException() throw()
        {};

    protected:

        // Constructor.
        SmcException(const std::string& reason)
        : std::runtime_error(reason)
        {};

    private:

        // Default construction not allowed.
        SmcException();

    //-----------------------------------------------------------
    // Member data.
    //
    public:
    protected:
    private:
    };

#ifdef SMC_FIXED_STACK
    class PushOnFullStateStackException :
        public SmcException
    {
    //-----------------------------------------------------------
    // Member methods.
    //
    public:

        // Default constructor.
        PushOnFullStateStackException()
        : SmcException("cannot push on full state stack")
        {};

        // Destructor.
        virtual ~PushOnFullStateStackException() throw()
        {};

    protected:
    private:

    //-----------------------------------------------------------
    // Member data.
    //
    public:
    protected:
    private:
    };
#endif

    // This class is thrown when a pop is issued on an empty
    // state stack.
    class PopOnEmptyStateStackException :
        public SmcException
    {
    //-----------------------------------------------------------
    // Member methods.
    //
    public:

        // Default constructor.
        PopOnEmptyStateStackException()
        : SmcException("no state to pop from state stack")
        {};

        // Destructor.
        virtual ~PopOnEmptyStateStackException() throw()
        {};

    protected:
    private:

    //-----------------------------------------------------------
    // Member data.
    //
    public:
    protected:
    private:
    };

    // This class is thrown when a transition is issued
    // but there is no current state. This happens when
    // a transition is issued from within a transition
    // action.
    class StateUndefinedException :
        public SmcException
    {
    //-----------------------------------------------------------
    // Member methods.
    //
    public:

        // Default constructor.
        StateUndefinedException()
        : SmcException("transition invoked while in transition")
        {};

        // Destructor.
        virtual ~StateUndefinedException() throw()
        {};

    protected:
    private:

    //-----------------------------------------------------------
    // Member data.
    //
    public:
    protected:
    private:
    };

    // This class is thrown when a transition is issued
    // but there is no code to handle it.
    class TransitionUndefinedException :
        public SmcException
    {
    //-----------------------------------------------------------
    // Member methods.
    //
    public:

        // Default constructor.
        TransitionUndefinedException()
        : SmcException("no such transition in current state"),
          _state(NULL),
          _transition(NULL)
        {};

        // Construct an exception using the specified state
        // and transition.
        TransitionUndefinedException(const char *state,
                                     const char *transition)
        : SmcException("no such transition in current state"),
          _state(copyString(state)),
          _transition(copyString(transition))
        {};

        // Copy constructor.
        TransitionUndefinedException(
            const TransitionUndefinedException& ex)
        : SmcException("no such transition in current state"),
          _state(copyString(ex._state)),
          _transition(copyString(ex._transition))
        {};

        // Destructor.
        virtual ~TransitionUndefinedException() throw()
        {
            if (_state != NULL)
            {
                delete[] _state;
                _state = NULL;
            }

            if (_transition != NULL)
            {
                delete[] _transition;
                _transition = NULL;
            }
        };

        // Assignment operator.
        const TransitionUndefinedException&
            operator=(const TransitionUndefinedException& ex)
        {
            // Don't do self assignment.
            if (this != &ex)
            {
                if (_state != NULL)
                {
                    delete[] _state;
                    _state = NULL;
                }

                if (_transition != NULL)
                {
                    delete[] _transition;
                    _transition = NULL;
                }

                _state = copyString(ex._state);
                _transition = copyString(ex._transition);
            }

            return (*this);
        };

        // Returns the state. May be NULL.
        const char* getState() const
        {
            return(_state);
        };

        // Returns the transition. May be NULL.
        const char* getTransition() const
        {
            return (_transition);
        };

    protected:
    private:

    //-----------------------------------------------------------
    // Member data.
    //
    public:
    protected:
    private:

        char *_state;
        char *_transition;
    };

    // This class is thrown when a state ID is either less than
    // the minimal value or greater than the maximal value.
    class IndexOutOfBoundsException :
        public SmcException
    {
    //-----------------------------------------------------------
    // Member methods.
    //
    public:

        // Default constructor.
        IndexOutOfBoundsException()
        : SmcException("index out of bounds"),
          _index(0),
          _minIndex(0),
          _maxIndex(0)
        {};

        // Constructs an exception using the specified index,
        // minimum index and maximum index.
        IndexOutOfBoundsException(const int index,
                                  const int minIndex,
                                  const int maxIndex)
        : SmcException("index out of bounds"),
          _index(index),
          _minIndex(minIndex),
          _maxIndex(maxIndex)
        {};

        // Copy constructor.
        IndexOutOfBoundsException(
            const IndexOutOfBoundsException& ex)
        : SmcException("index out of bounds"),
          _index(ex._index),
          _minIndex(ex._minIndex),
          _maxIndex(ex._maxIndex)
        {};

        // Destructor.
        virtual ~IndexOutOfBoundsException() throw()
        {};

        // Assignment operator.
        const IndexOutOfBoundsException&
            operator=(const IndexOutOfBoundsException& ex)
        {
            // Don't do self assignment.
            if (this != &ex)
            {
                _index = ex._index;
                _minIndex = ex._minIndex;
                _maxIndex = ex._maxIndex;
            }

            return (*this);
        };

        // Returns the out-of-bounds index.
        int getIndex() const
        {
            return(_index);
        };

        // Returns the minimum allowed index value.
        int getMinIndex() const
        {
            return (_minIndex);
        };

        // Returns the maximum allowed index value.
        int getMaxIndex() const
        {
            return (_maxIndex);
        };

    protected:
    private:

    //-----------------------------------------------------------
    // Member data.
    //
    public:
    protected:
    private:

        int _index;
        int _minIndex;
        int _maxIndex;
    };
#endif // !SMC_NO_EXCEPTIONS

//
// end of Exception Classes.
//---------------------------------------------------------------

    class State
    {
    //-----------------------------------------------------------
    // Member functions.
    //
    public:

        const char* getName() const
        {
            return (_name);
        };

        int getId() const
        {
            return (_stateId);
        }

    protected:

        State(const char *name, int stateId)
        : _name(NULL),
          _stateId(stateId)
        {
            if (name != NULL)
            {
                _name = copyString(name);
            }
            else
            {
                _name = copyString("NAME NOT SET");
            }
        };

        virtual ~State()
        {
#ifndef SMC_FIXED_STACK
            if (_name != NULL)
            {
                // Delete the string iff static memory is
                // *not* used.
                delete[] _name;
                _name = NULL;
            }
#endif
        };

    private:

        // Make the default and copy constructors private to
        // prevent their use.
        State() {};
        State(const State&) {};

    //-----------------------------------------------------------
    // Member data.
    //
    public:
    protected:

        // This state's printable name.
        char *_name;

        // This state's unique identifier.
        int _stateId;

    private:
    };

    class FSMContext
    {
    //-----------------------------------------------------------
    // Nested classes.
    //
    public:
    protected:
    private:

        // Implements the state stack.
        class StateEntry
        {
        //-------------------------------------------------------
        // Member functions.
        //
        public:
            StateEntry(State *state, StateEntry *next)
            : _state(state),
              _next(next)
            {};

            ~StateEntry()
            {
                _state = NULL;
                _next = NULL;
            };

            State* getState()
            {
                return(_state);
            };

            StateEntry* getNext()
            {
                return(_next);
            };

        protected:
        private:

        //-------------------------------------------------------
        // Member data.
        //
        public:
        protected:
        private:
            State *_state;
            StateEntry *_next;

        //-------------------------------------------------------
        // Friends
        //
            friend class FSMContext;
        }; // end of class StateEntry

    //-----------------------------------------------------------
    // Member functions
    //
    public:

        // Destructor.
        virtual ~FSMContext()
        {
#ifdef SMC_FIXED_STACK
            _transition = NULL;
#else // ! SMC_FIXED_STACK
            StateEntry *state;

            if (_transition != NULL)
            {
                delete[] _transition;
                _transition = NULL;
            }

            // But we did allocate the state stack.
            while (_state_stack != NULL)
            {
                state = _state_stack;
                _state_stack = _state_stack->_next;
                delete state;
            }
#endif // ! SMC_FIXED_STACK
        };

        // Comparison and assignment operators
        // Assignment operator
        FSMContext& operator=(const FSMContext& fsm)
        {
            // Don't do the assignment if the left and right
            // hand sides are the same object.
            if (this != &fsm)
            {
                _state = fsm._state;
            }

            return(*this);
        };

        // Starts the finite state machine running by executing
        // the initial state's entry actions.
        virtual void enterStartState()=0;

        // Exact same object (is it me?)
        int same(const FSMContext& fsm) const
        {
            return(this == &fsm);
        };

        // Returns the debug flag's current setting.
        bool getDebugFlag()
        {
            return(_debug_flag);
        };

        // Sets the debug flag. A true value means debugging
        // is on and false means off.
        void setDebugFlag(bool flag)
        {
            _debug_flag = flag;
            return;
        };

#ifdef SMC_USES_IOSTREAMS
        // Returns the stream to which debug output is written.
        std::ostream& getDebugStream()
        {
            return (*_debug_stream);
        };

        // Sets the debug output stream.
        void setDebugStream(std::ostream& debug_stream)
        {
            _debug_stream = &debug_stream;
            return;
        }
#endif // SMC_USES_IOSTREAMS

        // Is this state machine already inside a transition?
        // Yes if state is null.
        bool isInTransition() const
        {
            return(_state == NULL ? true : false);
        };

        // Returns the current transition's name.
        // Used only for debugging purposes.
        char* getTransition()
        {
            return (_transition);
        };

        // Saves away the transition name only if debugging
        // is turned on.
        void setTransition(const char *transition)
        {
#ifndef SMC_FIXED_STACK
            if (_transition != NULL)
            {
                delete[] _transition;
                _transition = NULL;
            }
#endif // ! SMC_FIXED_STACK

            _transition = copyString(transition);

            return;
        };

        // Clears the current state.
        void clearState()
        {
            _previous_state = _state;
            _state = NULL;
        };

        // Returns the state which a transition left.
        // May be NULL.
        State* getPreviousState()
        {
            return (_previous_state);
        }

        // Sets the current state to the specified state.
        void setState(const State& state)
        {
            // clearState() is not called when a transition has
            // no actions, so set _previous_state to _state in
            // that situation. We know clearState() was not
            // called when _state is not null.
            if (_state != NULL)
            {
                _previous_state = _state;
            }

            _state = const_cast<State *>(&state);

            if (_debug_flag == true)
            {
#ifdef SMC_USES_IOSTREAMS
                *_debug_stream << "ENTER STATE     : "
                               << _state->getName()
                               << std::endl;
#else
                TRACE("ENTER STATE     : %s\n",
                      _state->getName());
#endif // SMC_USES_IOSTREAMS
            }
        };

#ifdef SMC_FIXED_STACK
        // Returns true if the state stack is empty and false
        // otherwise.
        bool isStateStackEmpty() const
        {
            return (_state_stack_depth == 0);
        }

        // Returns the state stack's depth.
        int getStateStackDepth() const
        {
            return (_state_stack_depth);
        }

        // Push the current state on top of the state stack
        // and make the specified state the current state.
        void pushState(const State& state)
        {
#ifdef SMC_NO_EXCEPTIONS
            assert (_state_stack_depth < SMC_STATE_STACK_SIZE);
#else
            if (_state_stack_depth == SMC_STATE_STACK_SIZE)
            {
                throw PushOnFullStateStackException();
            }
#endif

            // Do the push only if there is a state to be pushed
            // on the stack.
            if (_state != NULL)
            {
                _state_stack[_state_stack_depth] = _state;
                ++_state_stack_depth;
            }

            _previous_state = _state;
            _state = const_cast<State *>(&state);

            if (_debug_flag == true)
            {
#ifdef SMC_USES_IOSTREAMS
                *_debug_stream << "PUSH TO STATE   : "
                               << _state->getName()
                               << std::endl;
#else
                TRACE("PUSH TO STATE   : %s\n",
                      _state->getName());
#endif // SMC_USES_IOSTREAMS
            }
        };

        // Make the state on top of the state stack the
        // current state.
        void popState()
        {
            // Popping when there was no previous push is an error.
#ifdef SMC_NO_EXCEPTIONS
            assert(_state_stack_depth > 0);
#else
            if (_state_stack_depth == 0)
            {
                throw PopOnEmptyStateStackException();
            }
#endif

            _previous_state = _state;
            --_state_stack_depth;
            _state = _state_stack[_state_stack_depth];

            if (_debug_flag == true)
            {
#ifdef SMC_USES_IOSTREAMS
                *_debug_stream << "POP TO STATE    : "
                               << _state->getName()
                               << std::endl;
#else
                TRACE("POP TO STATE    : %s\n",
                      _state->getName());
#endif // SMC_USES_IOSTREAMS
            }
        };

        // Remove all states from the state stack.
        void emptyStateStack()
        {
            _state_stack_depth = 0;
        };
#else // ! SMC_FIXED_STACK
        // Returns true if the state stack is empty and false
        // otherwise.
        bool isStateStackEmpty() const
        {
            return (_state_stack == NULL);
        }

        // Returns the state stack's depth.
        int getStateStackDepth() const
        {
            StateEntry *state_ptr;
            int retval;

            for (state_ptr = _state_stack, retval = 0;
                 state_ptr != NULL;
                 state_ptr = state_ptr->getNext(), ++retval)
                ;

            return (retval);
        }

        // Push the current state on top of the state stack
        // and make the specified state the current state.
        void pushState(const State& state)
        {
            StateEntry *new_entry;

            // Do the push only if there is a state to be pushed
            // on the stack.
            if (_state != NULL)
            {
                new_entry = new StateEntry(_state, _state_stack);
                _state_stack = new_entry;
            }

            _previous_state = _state;
            _state = const_cast<State *>(&state);

            if (_debug_flag == true)
            {
#ifdef SMC_USES_IOSTREAMS
                *_debug_stream << "PUSH TO STATE   : "
                               << _state->getName()
                               << std::endl;
#else
                TRACE("PUSH TO STATE   : %s\n",
                      _state->getName());
#endif // SMC_USES_IOSTREAMS
            }
        };

        // Make the state on top of the state stack the
        // current state.
        void popState()
        {
            StateEntry *entry;

            // Popping when there was no previous push is an error.
#ifdef SMC_NO_EXCEPTIONS
            assert(_state_stack != NULL);
#else
            if (_state_stack == NULL)
            {
                throw PopOnEmptyStateStackException();
            }
#endif // SMC_NO_EXCEPTIONS

            _previous_state = _state;
            _state = _state_stack->getState();
            entry = _state_stack;
            _state_stack = _state_stack->getNext();
            delete entry;

            if (_debug_flag == true)
            {
#ifdef SMC_USES_IOSTREAMS
                *_debug_stream << "POP TO STATE    : "
                               << _state->getName()
                               << std::endl;
#else
                TRACE("POP TO STATE    : %s\n",
                      _state->getName());
#endif // SMC_USES_IOSTREAMS
            }
        };

        // Remove all states from the state stack.
        void emptyStateStack()
        {
            StateEntry *state_ptr,
                       *next_ptr;

            for (state_ptr = _state_stack;
                 state_ptr != NULL;
                 state_ptr = next_ptr)
            {
                next_ptr = state_ptr->getNext();
                delete state_ptr;
            }

            _state_stack = NULL;
        };
#endif // ! SMC_FIXED_STACK

    protected:

        // Default constructor.
        FSMContext(const State& state)
        : _state(const_cast<State *>(&state)),
          _previous_state(NULL),
#ifdef SMC_FIXED_STACK
          _state_stack_depth(0),
#else
          _state_stack(NULL),
#endif
          _transition(NULL),
#ifdef SMC_USES_IOSTREAMS
          _debug_flag(false),
          _debug_stream(&std::cerr)
#else
          _debug_flag(false)
#endif // SMC_USES_IOSTREAMS
        {};

    private:

        // I don't believe that it makes sense to copy a
        // context. It may make sense to copy the application
        // class but the new object is *not* in the same
        // state as the old - the new object must start in
        // the FSM's initial state. Therefore, the copy
        // constructor is private in order to prevent it
        // being used.
        FSMContext(const FSMContext&)
        {};

    //-----------------------------------------------------------
    // Member data
    //
    public:
    protected:

        // The current state of the finite state machine.
        State *_state;

        // Remember which state a transition left.
        State *_previous_state;

        // The stack of pushed states.
#ifdef SMC_FIXED_STACK
        State* _state_stack[SMC_STATE_STACK_SIZE];
        int _state_stack_depth;
#else
        StateEntry *_state_stack;
#endif

        // The current transition *name*. Use for debugging
        // purposes.
        char *_transition;

    private:

        // When this flag is set to true, this class will print
        // out debug messages.
        bool _debug_flag;

// Include the following only if C++ iostreams are being used.
#ifdef SMC_USES_IOSTREAMS
        // When FSM debugging is on, debug messages will be
        // written to this output stream. This stream is set to
        // standard error by default.
        std::ostream *_debug_stream;
#endif // SMC_USES_IOSTREAMS

    }; // end of class FSMContext
}

//
// CHANGE LOG
// Log: statemap.h,v
// Revision 1.19  2014/09/06 19:31:28  fperrad
// remove hard tab
//
// Revision 1.18  2013/07/14 14:32:36  cwrapp
// check in for release 6.2.0
//
// Revision 1.17  2011/11/20 14:58:32  cwrapp
// Check in for SMC v. 6.1.0
//
// Revision 1.16  2010/09/11 19:09:38  fperrad
// remove \r from debug message
//
// Revision 1.15  2009/11/24 20:42:39  cwrapp
// v. 6.0.1 update
//
// Revision 1.14  2009/03/01 18:20:40  cwrapp
// Preliminary v. 6.0.0 commit.
//
// Revision 1.13  2008/05/20 18:31:12  cwrapp
// ----------------------------------------------------------------------
//
// Committing release 5.1.0.
//
// Modified Files:
//     Makefile README.txt smc.mk tar_list.txt bin/Smc.jar
//     examples/Ant/EX1/build.xml examples/Ant/EX2/build.xml
//     examples/Ant/EX3/build.xml examples/Ant/EX4/build.xml
//     examples/Ant/EX5/build.xml examples/Ant/EX6/build.xml
//     examples/Ant/EX7/build.xml examples/Ant/EX7/src/Telephone.java
//     examples/Java/EX1/Makefile examples/Java/EX4/Makefile
//     examples/Java/EX5/Makefile examples/Java/EX6/Makefile
//     examples/Java/EX7/Makefile examples/Ruby/EX1/Makefile
//     lib/statemap.jar lib/C++/statemap.h lib/Java/Makefile
//     lib/Php/statemap.php lib/Scala/Makefile
//     lib/Scala/statemap.scala net/sf/smc/CODE_README.txt
//     net/sf/smc/README.txt net/sf/smc/Smc.java
// ----------------------------------------------------------------------
//
// Revision 1.12  2007/12/28 12:34:40  cwrapp
// Version 5.0.1 check-in.
//
// Revision 1.11  2007/08/05 12:58:54  cwrapp
// Version 5.0.1 check-in. See net/sf/smc/CODE_README.txt for more information.
//
// Revision 1.10  2007/01/15 00:23:50  cwrapp
// Release 4.4.0 initial commit.
//
// Revision 1.9  2006/07/11 18:28:22  cwrapp
// Move SmcException::copyString() to a package-wide routine.
//
// Revision 1.8  2006/04/22 12:45:24  cwrapp
// Version 4.3.1
//
// Revision 1.7  2005/06/08 11:09:14  cwrapp
// + Updated Python code generator to place "pass" in methods with empty
//   bodies.
// + Corrected FSM errors in Python example 7.
// + Removed unnecessary includes from C++ examples.
// + Corrected errors in top-level makefile's distribution build.
//
// Revision 1.6  2005/05/28 18:44:13  cwrapp
// Updated C++, Java and Tcl libraries, added CSharp, Python and VB.
//
// Revision 1.2  2005/02/21 19:01:42  charlesr
// Changed State::_id to State::_stateId because of Object-C++
// reserved word conflict.
//
// Revision 1.1  2004/05/31 13:44:41  charlesr
// Added support for non-iostreams output.
//
// Revision 1.0  2003/12/14 20:37:49  charlesr
// Initial revision

#endif
