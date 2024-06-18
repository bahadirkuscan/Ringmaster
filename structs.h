struct Person{
    char *name;
    char *location; //default location is "NOWHERE"
    char **items; // item names
    int *amounts; // defines the amount of items (each item will have its amount on the same index)
    int item_count; // the total number of items
    int item_array_size; // size of items array
};



struct Action{
    char **subjects;
    char *mode;
    char **objects; // what subjects will buy or sell, where subjects will go to
    int *amounts; // if the mode is sell or buy how many of each object will be processed (each object will have its amount on the same index)
    int num_of_subjects; // the total number of subjects
    int num_of_objects; // the total number of objects
    int subj_array_size; // subjects array size
    int obj_array_size; // objects array size
    char *trader; // will be used only for "sell to" and "buy from" operations
};

struct Action_Sequence{
    struct Action **actions;
    int action_array_size; // actions array size
    int action_count; // total number of actions
};

struct Condition{
    char **subjects;
    char *mode;
    char **objects;
    int *amounts;
    int num_of_subjects;
    int num_of_objects;
    int subj_array_size;
    int obj_array_size;
    // Conditions does not have traders
};

struct Condition_Sequence{
    struct Condition **conditions;
    int condition_array_size;
    int condition_count;
};