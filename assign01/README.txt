Interesting notes on implementation:
    An interesting method for my implementation, which I believe could vary for others, is that I created
    a seperate map called "blocks" which represents all variables declared in the code. This was for the analyze function
    within the Interpreter. This essentially assumes that any code done will most likely use variables in its implementation, so
    storing it in a map that could be used in other functions proved useful, mainly when I recursively got to a variable reference
    in my execute function and I needed to get the value associated with that reference. I am using this same map between analyze and execute, which is why I have to clear between uses.

    In the analyze function, I had created my own pre order traversal which works in a similar way to the implementation within node.h. This was done so i could have additional checks during each recursive call.

    In the execute function, I had used a queue to recursively traverse the tree rather than using 'each kid' in node.h to apply operations to the children. This was merely for my own intuition and understanding during implementation, since using 'each kid' would result in the same outcome. 