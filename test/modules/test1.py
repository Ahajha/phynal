from module1 import add, identity, widen, narrow, nothing

print(identity(5))

print(identity(9999999))
print(identity(9999999))

print(identity(2**31 - 1))  # Passes
identity(2**31)  # Segfault

print(narrow(2**31 - 1))  # Passes
print(narrow(2**31))  # Passes

print(widen(2**31 - 1))  # Passes
print(widen(2**31))  # Segfault

nothing(2**31 - 1)
nothing(2**31)  # Segfault

# print(identity(99999999999999999999))  # Fails, too big to fit in long

# add(99999999, 99999999999999) # Fails, too big to fit in long
