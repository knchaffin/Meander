/*  Copyright (C) 2019-2024 Ken Chaffin
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "plugin.hpp"

extern void ConfigureGlobals();

Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelMeander);
	p->addModel(modelModeScaleQuant); 
	p->addModel(modelModeScaleProgressions); 

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.

}

