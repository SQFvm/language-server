
if (true) then {
    private _foo = 0;
    diag_log _foo;
};
diag_log _foo; // Should raise warning (not assigned)
private _foo = 1; // Should raise information (not used)

private _bar = 0;
if (true) then {
    diag_log _bar;
    _bar = 1;
};
diag_log _bar;

if (true) then {
    FooBarScopeCheck = 0;
    diag_log FooBarScopeCheck;
};
diag_log FooBarScopeCheck;