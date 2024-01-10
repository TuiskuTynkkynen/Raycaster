#pragma once

#include  <cstdint>

namespace Core {
	class ElementBuffer
	{
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	public:
		ElementBuffer(const uint32_t* data, uint32_t count);
		~ElementBuffer();

		void Bind() const;
		void Unbind() const;

		inline uint32_t GetCount() { return m_Count; }
	};
}

