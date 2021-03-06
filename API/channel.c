//
//  channel.c
//  NIolang
//
//  Created by Дмитрий Маслюков on 19/08/2019.
//  Copyright © 2019 Дмитрий Маслюков. All rights reserved.
//

#include "../api.h"
#include "../INCLUDE/dulthread.h"


METHOD_DECL(setCallback){
    if(Args.a_passed != 1){
        fprintf(stderr, "in setCallback 1 argument expected but %d passed", Args.a_passed);
        return 0;
    }
    object* arg = *Args.aptr;
    if(strcmp(arg->type->name, "functional object")!=0 && strcmp(arg->type->name, "builtin")!=0){
        fprintf(stderr, "passed argument not callable");
        return 0;
    }
    ((dulchannel*)self)->receive_callback = arg;
    return 0;
}

METHOD_DECL(push){
    dulchannel* self_ = (dulchannel*)self;
    if(self_->last - self_->first + Args.a_passed >= self_->capacity){
        self_->first = realloc(self_->first, (self_->capacity*=2)*sizeof(object*));
    }
    for(object** arg = Args.aptr; arg < Args.aptr+Args.a_passed; ++arg){
        *self_->last++ = *arg;
    }
    if(self_->receive_callback!=0){
        object* callback = self_->receive_callback;
        if(callback->type->type_id == func_id){
            context* c = init_context((funcobject*)callback, ctx->coroutine);
            object* to_ret = _mktuple(self_->last, (int)(self_->last - self_->first));
            self_->last = self_->first;
            for(int i = 0; i<((funcobject*)callback)->namecount; ++i){
                if(strcmp(((funcobject*)callback)->varnames[i], "this")==0){
                    c->vars[i] = self;
                    c->this_ptr = self;
                }
            }
            c->vars[0] = to_ret;
            INCREF(to_ret);
        }
        if(callback->type == &BINTYPE){
            bin_method* f = (bin_method*)callback;
            binarg Args;
            f->func_pointer(self, Args, 0);
        }
    }
    return 0;
}

METHOD_DECL(get){
    dulchannel* s = (dulchannel*)self;
    if(s->first == s->last){
        return 0;
    }
    object * to_ret = *s->first;
    memmove(s->first, s->first + 1, (s->last - s->first )*sizeof(object*));
    return to_ret;
}

METHOD_DECL(get_all){
    dulchannel* s = (dulchannel*)self;
    if(s->first == s->last){
        return 0;
    }
    object* to_ret = _mktuple(s->last, (int)(s->last - s->first));
    s->last = s->first;
    return to_ret;
}

BIN_DECL(new_channel){
    dulchannel* c = dulalloc(sizeof(dulchannel));
    c->capacity = 15;
    c->first = dulalloc(sizeof(object*)*c->capacity);
    c->last = c->first;
    c->refcnt = 0;
    c->type = &CHANTYPE;
    c->receive_callback = 0;
    return (object*)c;
}

METHOD_DECL(wait_all){
    dulchannel * s = (dulchannel*)self;
    if(s->first == s->last){
        ctx->inst_pointer--;
        return 0;
    }
    object* to_ret = _mktuple(s->last, (int)(s->last - s->first));
    s->last = s->first;
    return to_ret;
}

static struct {
    const char * name;
    bin_method m;
} chan_methods [] = {{
    "setCallback", {
        &BINTYPE,
        1,
        &setCallback
    }}, {
    "push", {
        &BINTYPE,
        1,
        &push
    }}, {
    "get",{
        &BINTYPE,
        1,
        &get
    }}, {
    "getAll", {
        &BINTYPE,
        1,
        &get_all
    }}, {
    "waitAll", {
        &BINTYPE,
        1,
        &wait_all
    }}
    
};
static int chan_mc = sizeof(chan_methods)/sizeof(*chan_methods);


object * get_chan_methods(void){
    static object * methods;
    if(methods == 0){
        methods = new_ob();
        for(int i = 0; i< chan_mc; ++i){
            ob_subscr_set(methods, strfromchar(chan_methods[i].name), (object*)&chan_methods[i].m);
        }
    }
    return methods;
}

char* dump_channel(object * c){
    dulchannel*self = (dulchannel*)c;
#warning TODO: decrease mallocs/ensafe
    int elemc = (int)(self->last - self->first);
    char** child_dump = (char**)dulalloc(sizeof(char*)*elemc);
    for(object** child = self->first; child != self->last; ++child){
        child_dump[child - self->first] = (*child)->type->dump((*child));
    }
    int final_size = 0;
    for(int i = 0; i<elemc; ++i){
        final_size += strlen(child_dump[i])+2;
    }
    char* selfdump = dulalloc(final_size + 3);
    selfdump[final_size + 2] = 0;
    char* writer = selfdump;
    *writer++ = '<';
    for(int i = 0; i<elemc; ++i){
        writer += sprintf(writer, "%s, ", child_dump[i]);
    }
    *writer='>';
    for(int i = 0; i< elemc; ++i){
        free(child_dump[i]);
    }
    free(child_dump);
    return selfdump;
}

const struct obtype CHANTYPE = {
    "channel",
    &dump_channel, //dump
    0, //alloc
    0, //dealloc
    0, //+
    0, //-
    0, // *
    0, // /
    0, // +=
    0, // -=
    0, // *=
    0, // /=
    0, // <
    0, // >
    0, // ==
    0, // <=
    0, // >=
    0, // f()
    0, // a in b
    0, //init_iter (collection initializes iter)
    0, //next_iter
    0, // [0]
    0, // [0] =
    0, // tostr
    0, //copy
    0,  //unpack,
    -1, //typeid
    &get_chan_methods// method
};
