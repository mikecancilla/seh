// seh.cpp : Defines the entry point for the console application.
//
// See: https://cristianadam.eu/20160914/nullpointerexception-in-c-plus-plus/
//

#ifdef USE_WINDOWS_SE
#include "stdafx.h"

#include <eh.h>
#include <windows.h>
#else
#include <signal.h>
#endif

#include <memory>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <functional>
#include <map>

#include "except.hpp"

struct Message
{
    std::string message;
    Message(const std::string& aMessage) : message(aMessage)
    {
        std::cout << "Message: " << message << std::endl;
    }
    
    ~Message()
    {
        std::cout << "~Message: " << message << std::endl;
    }
};

void readNullPointer()
{
    try
    {
       Message msg("read from nullptr");
       int* p = nullptr;
       std::cout << *p << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }
}

void writeNullPointer()
{
   try
   {
      Message msg("write to nullptr");
      int* p = nullptr;
      *p = 42;
      std::cout << *p << std::endl;
   }
   catch (const std::exception& ex)
   {
       std::cout << ex.what() << std::endl;
   }
}

void divisionByZero()
{
   try
   {
      Message msg("division by zero");
      int a = 42;
      volatile int b = 0;
      std::cout << a / b << std::endl;
   }
   catch (const std::exception& ex)
   {
       std::cout << ex.what() << std::endl;
   }
}

void outOfBoundsVector()
{
    try
    {
        Message("out of bounds vector");
        std::vector<int> v;
        v[0] = 42;
        std::cout << v[0] << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }
}

void nullSharedPointer()
{
    try
    {
        Message("reading empty shared_ptr");
        std::shared_ptr<int> sp = std::make_shared<int>(42);
        std::shared_ptr<int> sp2;
        sp.swap(sp2);
        
        std::cout << *sp << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }
}

#if defined(USE_POSIX_SIGNAL) || defined(USE_POSIX_SIGACTION)
const char* signalDescription(int sgn)
{
    switch(sgn)
    {
        case SIGABRT: return "SIGABRT";
        case SIGFPE:  return "SIGFPE";
        case SIGILL:  return "SIGILL";
        case SIGINT:  return "SIGINT";
        case SIGSEGV: return "SIGSEGV";
        case SIGTERM: return "SIGTERM";
        default:      return "UNKNOWN";
    }
}
#endif

#ifdef USE_POSIX_SIGACTION    
void signalHandler(int sgn, siginfo_t *info, void *)
{
    if (sgn == SIGSEGV && info->si_addr == 0)
    {
        throw except::null_pointer_exception();
    }

    if (sgn == SIGFPE && (info->si_code == FPE_INTDIV || info->si_code == FPE_FLTDIV))
    {
        throw except::division_by_zero_exception();
    }

    std::ostringstream os;
    os << "Signal caught: " << signalDescription(sgn) << "(" << sgn << ")";

    throw std::runtime_error(os.str().c_str());
}

void register_for_os_exceptions()
{
    struct sigaction act;

    act.sa_sigaction = signalHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO | SA_NODEFER;

    sigaction(SIGABRT, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
}
#endif

#ifdef USE_POSIX_SIGNAL
void
signalHandler(int sgn)
{
    std::ostringstream os;
    os << "Signal caught: " << signalDescription(sgn) << "(" << sgn << ")";

    signal(sgn, signalHandler);

    throw std::runtime_error(os.str().c_str());
}

void register_for_os_exceptions()
{
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);
}
#endif

#ifdef USE_WINDOWS_SE
const char* seDescription(const unsigned int& code)
{
    switch (code)
    {
        case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
        case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
        case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
        default:                                 return "UNKNOWN EXCEPTION";
    }
}
    
void seTranslator(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
    if (code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_IN_PAGE_ERROR)
    {
        if (ep->ExceptionRecord->ExceptionInformation[1] == 0)
        {
            throw except::null_pointer_exception();
        }
    }
    else if (code == EXCEPTION_FLT_DIVIDE_BY_ZERO ||
                code == EXCEPTION_INT_DIVIDE_BY_ZERO)
    {
        throw except::division_by_zero_exception();
    }

    std::ostringstream os;
    os << "Structured exception caught: " << seDescription(code);

    throw std::runtime_error(os.str().c_str());
}

void register_for_os_exceptions()
{
    _set_se_translator(seTranslator);
}
#endif

std::vector<std::function<void()>> processArguments(int argc, char* argv[])
{
    std::vector<std::string> arguments(argv, argv + argc);

    std::map<std::string, std::function<void()>> functions
    {
        { "readNullPointer", readNullPointer },
        { "writeNullPointer", writeNullPointer },
        { "nullSharePointer", nullSharedPointer },
        { "outOfBoundsVector", outOfBoundsVector },
        { "divisionByZero", divisionByZero }
    };

    std::vector<std::function<void()>> callList;

    if (arguments.size() == 1)
    {
        std::ostringstream os;
        for (auto pair : functions)
        {
            if (os.str().size())
            {
                os << "|";
            }
            os << pair.first;
        }
        std::cout << "Usage: " << arguments[0] << " [all][" << os.str() << "]" << std::endl;
    }
    else if (arguments.size() == 2 && arguments[1] == "all")
    {
        for (auto pair : functions)
        {
            callList.push_back(pair.second);
        }
    }
    else
    {
        for (auto arg : arguments)
        {
            auto it = functions.find(arg);
            if (it != functions.end())
            {
                callList.push_back(it->second);
            }
        }
    }

    return callList;
}

void terminateHandler()
{
    if (std::current_exception())
    {
        try
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            std::cout << "terminateHandler: " << ex.what() << std::endl;
        }
        catch (...)
        {
            std::cout << "terminateHandler: Unknown exception!" << std::endl;
        }
    }
    else
    {
        std::cout  << "terminateHandler: called without an exception." << std::endl;
    }
    std::abort();
}

int main(int argc, char* argv[])
{
 //   except::register_for_os_exceptions();
    register_for_os_exceptions();
    std::set_terminate(terminateHandler);
   
    auto callList = processArguments(argc, argv);
//    for (int i = 0; i < 10 && callList.size(); ++i)
    for (int i = 0; i < 1 && callList.size(); ++i)
    {
        std::cout << i << "------------------------------------" << std::endl;
        for (auto func : callList)
        {
            func();
        }
        std::cout << "------------------------------------" << i << std::endl;
    }
}