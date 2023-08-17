// _x never assigned
private ["_ctrlImport", "_import"];
{
    _ctrlImport lbAdd (_x select 0);
} forEach _import;

// _forEachIndex never assigned, _x never assigned
private ["_baseRadios", "_channels", "_volumes", "_spatials", "_radios"];
acre_api_fnc_getBaseRadio = nil;
acre_api_fnc_getRadioChannel = nil;
acre_api_fnc_getRadioVolume = nil;
acre_api_fnc_getRadioSpatial = nil;
{
    // Loop through maximum 6 radios
    if (_forEachIndex >= 6) exitWith {};
    _baseRadio = [_x] call acre_api_fnc_getBaseRadio;
    _baseRadios pushBack _baseRadio;
    _channel = [_x] call acre_api_fnc_getRadioChannel;
    _channels pushBack _channel;
    _volume = [_x] call acre_api_fnc_getRadioVolume;
    _volumes pushBack _volume;
    _spatial = [_x] call acre_api_fnc_getRadioSpatial;
    _spatials pushBack _spatial;
} forEach _radios;

//_this never assigned
KPCF_ace = nil;
KPCF_fnc_manageAceActions = nil;
KPCF_interactRadius = nil;
KPCF_fnc_openDialog = nil;
private ["_cfBase"];
if (KPCF_ace) then {
    [_cfBase] call KPCF_fnc_manageAceActions;
} else {
    _cfBase addAction [
        "<t color='#FF8000'>" + localize "STR_KPCF_ACTIONOPEN" + "</t>", {
            [_this] call KPCF_fnc_openDialog;
        },
        nil,
        1,
        false,
        true,
        "",
        "true",
        KPCF_interactRadius
    ];
};

// _i should not raise an error
KPCF_fnc_getConfigPath = nil;
KPCF_inventory = nil;
private ["_count", "_cargo", "_config"];
for "_i" from 0 to (_count - 1) do {
    _config = [(_cargo select 0) select _i] call KPCF_fnc_getConfigPath;
    KPCF_inventory pushBack [
        (getText (configFile >> _config >> ((_cargo select 0) select _i) >> "displayName")),
        (_cargo select 0) select _i,
        (_cargo select 1) select _i
    ];
};