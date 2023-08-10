private _variable = "test";
Variable = "test";

isNil "_variable";
diag_log format["getVariable global", missionNamespace getVariable "variable"];
[] call {
    diag_log format["call local", _Variable];
    diag_log format["call global", missionNamespace getVariable "variable"];
};
[] spawn {
    diag_log format["spawn local", _variable];
    diag_log format["spawn global", missionNamespace getVariable "Variable"];
};
isNil {
    diag_log  format["isNil local", _vaRiable];
    diag_log  format["isNil global", missionNamespace getVariable "Variable"];
};