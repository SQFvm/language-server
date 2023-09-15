{
    diag_log _x;
} forEach [1, 2, 3];
    
{
    private _value = _x;
    diag_log _value;
} forEach [1, 2, 3],

{
    value = _x;
    diag_log value;
} forEach [1, 2, 3];