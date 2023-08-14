// Add units
_valids = allUnits select {
    (alive _x)                                                                              // Alive
    && {
        (KP_liberation_enemies_zeus && {!(side (group _x) isEqualTo GRLIB_side_civilian)})  // Not civilian side, if enemy adding is enabled
        || {side (group _x) isEqualTo GRLIB_side_friendly}                                  // Player side if enemy adding is disabled
    }
    && {((str _x) find "BIS_SUPP_HQ_") isEqualTo -1}                                        // Not a HQ entity from support module
};

// Add vehicles
_valids append (vehicles select {
    (alive _x)                                                                              // Alive
    && {
        ((toLower (typeOf _x)) in _vehicleClassnames)                                       // In valid classnames
        || (_x getVariable ["KPLIB_captured", false])                                       // or captured
        || (_x getVariable ["KPLIB_seized", false])                                         // or seized
    }
    && {isNull (attachedTo _x)}                                                             // Not attached to something
});