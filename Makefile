CC = g++
TARGET = oams
HOME_PATH=.

COMMON_PATH=/home/mcpas
COMMON_BIN_PATH=$(COMMON_PATH)/bin
COMMON_LIB_PATH=-L$(COMMON_PATH)/lib
COMMON_INC_PATH=-I$(COMMON_PATH)/include
COMMON_CFG_PATH=$(COMMON_PATH)/cfg/EMS0

DBLIB=-L$(ALTIBASE_HOME)/lib -lalticapi -lodbccli 
DBINC=-I$(ALTIBASE_HOME)/include

LFLAGS=-lpthread -ldl -lrt -std=c++11 -lsmqueue64 -lbtxbus3e-64 -lm -lcrypto -lnet64 -lsmcom64 -lShmOAMSv5
GO_LIB=/home/mcpas/lib/exporter_lib/libprom_metrics.so

DEFINES=-DONESERVER_MODE -DOFCS_IN_EMS -DALTIBASE_MODE -DASv5
SRCS = OAMS_main.cpp OAMS_common.cpp OAMS_data.cpp LIB_nsmlog.cpp OAMS_config.cpp OAMS_mgm.cpp OAMS_exporter.cpp OAMS_altiCInf.cpp OAMS_db.cpp OAMS_limit.cpp OAMS_alarm.cpp OAMS_monitor.cpp \
       OAMS_xbus.cpp OAMS_extern.cpp OAMS_stat.cpp OAMS_hwmon.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(DBLIB) $(LFLAGS) $(COMMON_LIB_PATH) $(LDFLAGS) $(GO_LIB)

%.o: %.cpp
	$(CC) $(DBLIB) $(LFLAGS) $(DBINC) $(COMMON_INC_PATH) $(COMMON_LIB_PATH) $(DEFINES) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
