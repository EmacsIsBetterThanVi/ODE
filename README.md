# ODE
Ode is an interpreted, dynamicaly typed programming language writen in C++, and designed to be able to be compiled in some situations as well.
Ode is mostly writen as low level funtions, which are then called using higher level functions and classes.
## Language
Ode is built around a modal of low level functions performing everything. At its base, Ode has only a few behaviors, all of which are syntactical. Ode however defines a bunch of specifc variables which do specific actions when changed. The behaviors are: <code>name(...)</code> calls function named "name" from the scope. <code>name{...}</code> defines an object in the specified scope with members in the brackets. <code>name:type{...}</code> defines an object in the specified scope parented by type with members in the brackets. In Ode everything is an object, so to define an integer named x with value 1, one need only do <code>x{1}</code>. Not defining a key means that it is not a member, but rather a direct value, thus making it a primitive instead of an object. Thus
```
x{1}
print(x)
```
would print 1. In Objects, members are listed like <code>{key1: value1, key2: value2}</code>. An array object can be defined by listing multiple non-keyed values. 

Scope specifiers are nessisary when defining variables, since all code is executed at the local scope. A variable by default is accsesed from and modified in the second most restricitve scope: inherit. Thus, in order to interact with the other scopes, except for accessing, one needs to use scope specifiers. These are writen as <code>name\<scope\></code>. For example, the global integer variable x could be defined using <code>x<global>{1}</code>, but provided it is not shadowed, can be accessed without the specifier. If shadowed, then the access specifier will unshadow it. Valid scopes are, in order of restrictivness: Local (only available in the current context), inherit (only available in the current and child contexts), object (only available in the object, or by access as a member), thread (only available in contexts using this thread), and global (Available in all contexts). Variables are assumed to be object while inside an object. Variables defined by the system with specific uses are all marked with a __ (Double underscore) at the begining, such as __stdout, a string variable defined as <code>__stdout<global>{""}</code>.
