MAIN_IGNORE := ./TEST_F407ZG/%
MAIN_IGNORE += ./testframework/%
MAIN_IGNORE += ./libs/ChaNFS/%
MAIN_IGNORE += ./libs/USBHost/%

ifeq "$(NONETWORK)" "1"
MAIN_IGNORE += ./libs/Network/%
DEFINES += -DNONETWORK
endif

ifeq "$(DISABLESD)" "1"
DEFINES += -DDISABLESD
endif

ifeq "$(DISABLEUSB)" "1"
# MAIN_IGNORE += ./libs/USBDevice/%
DEFINES += -DDISABLEUSB
endif

ifeq "$(DISABLEMSD)" "1" 
DEFINES += -DDISABLEMSD
endif

ifneq "$(DEVICE)" "LPC1768"
MAIN_IGNORE += ./libs/LPC17xx/%
endif

# Totally exclude any modules listed in EXCLUDE_MODULES
# uppercase function
uc = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))
EXL_MODULES = $(patsubst %, ./modules/%/%,$(EXCLUDED_MODULES))
MAIN_IGNORE += $(EXL_MODULES)
DEFINES += $(call uc, $(subst /,_,$(patsubst %,-DNO_%,$(EXCLUDED_MODULES))))