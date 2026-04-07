# ODE
Ode is an interpreted, dynamicaly typed programming language writen in C++, and designed to be able to be compiled in some situations as well.
Ode is mostly writen as low level funtions, which are then called using higher level functions and classes.
## Basic Language
Ode is built around a modal of low level functions performing everything. At its base, Ode has only a few behaviors, all of which are syntactical. Ode however defines a bunch of specifc variables which do specific actions when changed. The behaviors are: <code>name(args...)</code> calls function named "name" from the scope. <code>name{...}</code> defines an object in the specified scope with members in the brackets. <code>name:type{...}</code> defines an object in the specified scope parented by type with members in the brackets. In Ode everything is an object, so to define an integer named x with value 1, one need only do <code>x{1}</code>. Not defining a key means that it is not a member, but rather a direct value, thus making it a primitive instead of a struct. Thus
```
x{1}
print(x)
```
would print 1. In structs, members are listed like <code>{key1{value1}, key2{value2}}</code>. An array can be defined by listing multiple non-keyed values.

Scope specifiers are nessisary when defining variables, since all code is executed at the local scope. A variable by default is accsesed from and modified in the second most restricitve scope: inherit. Thus, in order to interact with the other scopes, except for accessing, one needs to use scope specifiers. These are writen as <code>name\<scope\></code>. For example, the global integer variable x could be defined using <code>x<global>{1}</code>, but provided it is not shadowed, can be accessed without the specifier. If shadowed, then the scope specifier will unshadow it. Valid scopes are, in order of restrictivness: Local (only available in the current context), inherit (only available in the current and child contexts), object (only available in the object, or by access as a member), thread (only available in contexts using this thread), and global (Available in all contexts). Variables are assumed to be object while inside an object. Variables defined by the system with specific uses are all marked with a __ (Double underscore) at the begining, such as __stdout, a string variable defined as <code>__stdout<global>{""}</code>. Reassignment is done the same way as definition, thus, to reasign x to have the value 3, one would use <code>x{3}</code>. The type will stay the same, thus 
```
x:int{1}
print(type(x))
x{x+1}
print(type(x))
```
will print 
```
_prim:int
_prim:int
```
Trying to modify the type, such as by
```
x:int{1}
x:string{"a"}
```
will throw a fatal error. Use of the <inhert> or <object> specifier will force definition in decending order
## Type
Each variable has a type, which can be obtained using the builtin type(name). This will return the type of the object, or on a non-object variable will return __prim for primitives, __struct for structs, __array for arrays, __class for classes, and __func for functions. Mixed types, such as object primitives(Eg. <code>x:int{1}</code>) will return <code>__prim:int</code>. An object can have any arbitrary extra variables, in additon to those defined by the constructor, and multiple types can be included. A constructor is defined in the class, and called simply by naming the type in the definition.
## Function and Class Definiton
Functions are defined as <code>name(args...){code...}</code> where args is a comma seperated list of arguments listed as name, or name:type. Constuctors  are a special type of function defined in an object using the syntax name(){code...}{variables...} where name is the same as the class. Classes are defined using a long structure of 
```
class name:parent{
  members...
}
```
If :parent is not included, then the class does not inhert any behaviors, and only the things defined in it are included. This is discourged as the object must implement all __ methods on its own. Technicaly a callable can be created using name(args...){code...}{variables...}, exactly like a constructor with arguments, which without addional types being added has a type of __func:__struct and behaves like both. This could be considred a primitive object, however it does not become a type(Thus it is final and uninstatiable), and can not have a constuctor. 
