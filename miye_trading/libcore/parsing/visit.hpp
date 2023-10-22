/*
 * Purpose: Macros to simplify making structs visitable
 */
#pragma once

#ifndef VISITOR
#define VISITOR \
    static constexpr bool visitable{true}; \
    template<typename This, typename Visitor> \
    static void visit(This *t, Visitor *v)
#endif

// visit mangled below allows a diffent name to the class member
#ifndef VISIT
#define VISIT(mem) v->visit(#mem, t->mem)
#endif
#ifndef VISIT_MANGLED
#define VISIT_MANGLED(name, mem) v->visit(name, t->mem)
#endif

/*
 * used like so:
 *
 * struct foo
 * {
 *     VISITOR
 *     {
 *         VISIT(some_member);
 *         VISIT_MANGLED("different_name", another_member);
 *     }
 *     some_type_t some_member;
 *     int         another_member;
 * };
 *
 *
 * generates code that looks like this:
 * cpp on the source to see it.
 *
 *
 *
 * template<typename This, typename Visitor>
 * static void visit(This* t, Visitor *v)
 * {
 *     v->visit("some_member", t->some_member);
 *     v->visit("different_name", t->another_member);
 * }
 */


// The below struct is a wrapper utility for the use of c++ lambdas to be used
// with the visit function. 
template <typename T>   
struct visit_wrapper{
    T visit;
    visit_wrapper(T& v):visit(v){}
};
template <typename T>
inline visit_wrapper<T> make_visitor_wrapper(T& lambda) { return visit_wrapper<T>(lambda); }


