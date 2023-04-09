RWStructuredBuffer<int> Numbers: register(u0);

[numthreads(32, 1, 1)]
void main(uint3 thread_id: SV_DISPATCHTHREADID)
{
	Numbers[thread_id.x] += 1;
}