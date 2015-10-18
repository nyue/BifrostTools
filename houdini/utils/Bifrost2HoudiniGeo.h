#pragma once

#include <string>

class Bifrost2HoudiniGeo
{
public:
	Bifrost2HoudiniGeo(const std::string& i_bifrost_filename,const std::string& i_hougeo_filename);
	virtual ~Bifrost2HoudiniGeo();
	virtual bool process();
private:
	std::string _bifrost_filename;
	std::string _hougeo_filename;
};
// == Emacs ================
// -------------------------
// Local variables:
// tab-width: 4
// indent-tabs-mode: t
// c-basic-offset: 4
// end:
//
// == vi ===================
// -------------------------
// Format block
// ex:ts=4:sw=4:expandtab
// -------------------------
