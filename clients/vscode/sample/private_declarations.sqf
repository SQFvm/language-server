private _keyword = 0;
_keyword + 1;

private "_string";
_string + 1; 

private ["_array"];
_array + 1;

for "_i" from 0 to 1 do { _i + 1; };
{ _x + 1; } count [];
{ _x + 1; } forEach [];
[] apply { _x + 1; };
[] findIf { _x + 1; };
[] select { _x + 1; };

params ["_params_a", ["_params_b", 5]];
_params_a + _params_b;

private ["_a", "_b"];
if (_a) then
{
    systemChat [_b];
};