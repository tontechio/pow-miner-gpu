On old AMD OpenCL versions, you might need something like the following:

```shell
cmake -DMINEROPENCL=1 -DOpenCL_LIBRARY=/opt/AMDAPPSDK/AMDAPPSDK-3.0/lib/x86_64/libOpenCL.so ..
```
