private _item = getItemCargo KPCF_activeStorage;
private _weapon = getWeaponCargo KPCF_activeStorage;
private _magazine = getMagazineCargo KPCF_activeStorage;
private _backpack = getBackpackCargo KPCF_activeStorage;
private _cargo = _item;
(_cargo select 0) append (_weapon select 0);
(_cargo select 1) append (_weapon select 1);
(_cargo select 0) append (_magazine select 0);
(_cargo select 1) append (_magazine select 1);
(_cargo select 0) append (_backpack select 0);
(_cargo select 1) append (_backpack select 1);

// Count the variable index
private _count = count (_cargo select 0);

private _config = "";

// Adapt the cargo into KPCF variable 
for "_i" from 0 to (_count-1) do {
    _config = [(_cargo select 0) select _i] call KPCF_fnc_getConfigPath;
    KPCF_inventory pushBack [
        (getText (configFile >> _config >> ((_cargo select 0) select _i) >> "displayName")),
        (_cargo select 0) select _i,
        (_cargo select 1) select _i
    ];
};