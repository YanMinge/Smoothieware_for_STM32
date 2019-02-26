/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CIRCBUFFER_H
#define CIRCBUFFER_H

template <class T, int Size>
class CircBuffer {
public:
    CircBuffer():write(0), read(0){}
    bool isFull() {
        return ((write + 1) % size == read);
    };

    bool isEmpty() {
        return (read == write);
    };

    void queue(T k) {
        if (isFull()) {
            read++;
            read %= size;
        }
        buf[write++] = k;
        write %= size;
    }

/* Begin by Yanminge 2019-01-25 */
#if SMOOTHIEWARE_FEATURE_ENABLE
    // pop last entered character
    void pop() {
        if(!isEmpty()) {
            write--;
            write %= size;
        }
    }

    void flush() {
        read = write;
    }

    uint16_t free() {
        return size - available() - 1;
    };

#endif /* SMOOTHIEWARE_FEATURE_ENABLE */
/* End by Yanminge 2019-01-25 */

    uint16_t available() {
        return (write >= read) ? write - read : size - read + write;
    };

    bool dequeue(T * c) {
        bool empty = isEmpty();
        if (!empty) {
            *c = buf[read++];
            read %= size;
        }
        return(!empty);
    };

private:
    volatile uint16_t write;
    volatile uint16_t read;
    static const int size = Size+1;  //a modern optimizer should be able to remove this so it uses no ram.
    T buf[Size+1];
};

#endif
