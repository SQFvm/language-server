private _declaration = nil;
_declaration = 1;
diag_log _declaration;

private _unused = 1;
_unused = 2;
diag_log _unused;

private _item = 1;
private _notAssigned = _item;
diag_log _notAssigned;

private ["_tst1"];
_tst1 = 123;
diag_log _tst1;

private ["_tst2"];
_tst2 = 123;
diag_log _tst2;