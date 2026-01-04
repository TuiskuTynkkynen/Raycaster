#pragma once

#include "Layer.h"

#include <vector>

namespace Core {
	class LayerStack {
	private:
		std::vector<Layer*> m_Layers;
		size_t m_InsertIndex;
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
	};
}