#pragma once

namespace Blainn
{
	struct Buffer
	{
		void* Data = nullptr;
		UINT64 Size = 0;

		Buffer() = default;

		Buffer(const void* data, UINT64 size = 0)
			: Data((void*)data), Size(size) {}

		static Buffer Copy(const Buffer& other)
		{
			Buffer buffer;
			buffer.Allocate(other.Size);
			memcpy(buffer.Data, other.Data, other.Size);
			return buffer;
		}

		static Buffer Copy(const void* data, UINT64 size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(UINT64 size)
		{
			delete[] (byte*)Data;
			Data = nullptr;
			Size = size;

			if (Size == 0)
				return;

			Data = new byte[size];
		}

		void Release()
		{
			delete[](byte*)Data;
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialize()
		{
			if (Data)
				memset(Data, 0, Size);
		}

		template<typename T>
		T& Read(UINT64 offset = 0)
		{
			return *(T*)((byte*)Data + offset);
		}

		template<typename T>
		const T& Read(UINT64 offset = 0) const
		{
			return *(T*)((byte*)Data + offset);
		}

		byte* ReadBytes(UINT64 size, UINT64 offset) const
		{
			assert(offset + size <= Size && "Buffer overflow!");
			byte* buffer = new byte[size];
			memcpy(buffer, (byte*)Data + offset, size);
			return buffer;
		}

		void Write(const void* data, UINT64 size, UINT64 offset = 0)
		{
			assert(offset + size <= Size && "Buffer overflow!");
			memcpy((byte*)Data + offset, data, size);
		}

		operator bool() const
		{
			return (bool)Data;
		}

		byte& operator[](int index)
		{
			return ((byte*)Data)[index];
		}

		byte& operator[](int index) const
		{
			return ((byte*)Data)[index];
		}

		template<typename T>
		T* As() const
		{
			return (T*)Data;
		}

		inline UINT64 GetSize() const { return Size; }
	};

	struct BufferSafe : public Buffer
	{
		~BufferSafe()
		{
			Release();
		}

		static BufferSafe Copy(const void* data, uint64_t size)
		{
			BufferSafe buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}
	};
}
