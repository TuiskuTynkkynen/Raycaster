#include "LayerStack.h"

#include <algorithm>

namespace Core {
	LayerStack::LayerStack(){
		m_InsertIndex = 0;
	}
	
	LayerStack::~LayerStack(){
		for (Layer* layer : m_Layers) {
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer){
		m_Layers.emplace(m_Layers.begin() + m_InsertIndex, layer);
		m_InsertIndex++;
	}
	
	void LayerStack::PopLayer(Layer* layer){
		std::vector<Layer*>::iterator iterator = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (iterator != m_Layers.end()) {
			layer->SetEnabled(false);
			m_Layers.erase(iterator);
			m_InsertIndex--;
		}

	}
	
	void LayerStack::PushOverlay(Layer* overlay){
		m_Layers.emplace_back(overlay);
	}
	
	void LayerStack::PopOverlay(Layer* overlay){
		std::vector<Layer*>::iterator iterator = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (iterator != m_Layers.end()) {
			overlay->SetEnabled(false);
			m_Layers.erase(iterator);
			m_InsertIndex--;
		}
	}
}
