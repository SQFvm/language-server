#include "include.h"
#include "/pboprefix/sample/include.h"
#define TEST
#define CFUNC(ARG) ARG
testvalue = "";



systemChat CONCAT(test,value);

private _something = [];
private _input = _this select 0;
[1, 2, 3] params ["_one", "_two", ["_three", 3]];

// Reserved forEach variables
{
    _x;
    _forEachIndex;
} forEach _something;
 
path = "a3\air_f\config.cpp";
private "_code";
private ["_state", "_args"];
private ["_return"];

_state = _state getVariable ["##state", "init"];
_args = _state getVariable ["##args", []];
_code = _state getVariable [_state, { "init" }];
_return = _args call _code;
private _someRandomVar = 1; //noprivate
private _anotherVar = 2;
private _avar = 2;

// -- Bracket spacing
if (true) then {

};

// -- Exit the machine and destroy it, run any exit code if it exists (It doesn't by default)
if (_newState isEqualTo "exit") exitWith
{
    private _exitCode = _state getVariable ["exit", {}];
    if !(_exitCode isEqualTo {}) then
    {
        _args call _exitCode;
    }
    else
    {
        systemChat "Do something";
    };
    if (_something) then
    {
     
    };
    [_state] call CFUNC(macroFunction);
}; 

// Local function declaration
private _fnc_something = {
    diag_log "Doing stuff";
};

for "_i" from 0 to 20 do
{
    diag_log str [_i];
};

_state setVariable ["##state", _newState];
_state setVariable ["##args", _args];
private _variable = "something";

[] call fn_tow_canHitch;
[] call fn_tow_towParent;