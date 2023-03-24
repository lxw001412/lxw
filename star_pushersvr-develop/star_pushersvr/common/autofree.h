#pragma once

#define AutoFree(className, instance) \
impl_AutoFree<className> _auto_free_##instance(&instance, false)
#define AutoFreeA(className, instance) \
impl_AutoFree<className> _auto_free_array_##instance(&instance, true)

template<class T>
class impl_AutoFree
{
public:
    impl_AutoFree(T** p, bool array) 
    {
        ptr = p;
        is_array = array;
    }

    virtual ~impl_AutoFree() 
    {
        if (ptr == NULL || *ptr == NULL) 
        {
            return;
        }

        if (is_array) 
        {
            delete[] * ptr;
        }
        else 
        {
            delete *ptr;
        }

        *ptr = NULL;
    }

private:
    T** ptr;
    bool is_array;
};