#pragma once

class SQDrawDelegate
{
public:
    SQDrawDelegate() {}
    virtual ~SQDrawDelegate() {}
    
    virtual void draw() = 0;
    virtual void resize(int x, int y) = 0;
    
};