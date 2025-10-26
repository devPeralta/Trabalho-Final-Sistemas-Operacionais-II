# compilador para C++
CXX = g++

# Flags do compilador
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Ishared -Iserver_files -Iclient_files

# -lpthread linka a biblioteca posix Thrreads
LDFLAGS = -lpthread

SHARED_DIR = shared
SERVER_DIR = server_files
CLIENT_DIR = client_files

TARGET_SERVER = servidor
TARGET_CLIENT = cliente

# Arquivos fonte (.cpp) 
# Fontes comuns ao cliente e servidor
SHARED_SRCS = $(SHARED_DIR)/utils.cpp

# Fontes do Servidor
SERVER_SRCS = $(SERVER_DIR)/server.cpp \
              $(SERVER_DIR)/subservico_descoberta.cpp \
              $(SERVER_DIR)/subservico_processamento.cpp \
              $(SERVER_DIR)/subservico_interface.cpp

# Fontes do Cliente
CLIENT_SRCS = $(CLIENT_DIR)/client.cpp \
			  $(CLIENT_DIR)/subservico_network_cli.cpp \
	 		  $(CLIENT_DIR)/subservico_input_cli.cpp \
			  $(CLIENT_DIR)/subservico_descoberta_cli.cpp

# Gera nomes de .o a partir dos .cpp
SHARED_OBJS = $(SHARED_SRCS:.cpp=.o)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)


.PHONY: all
all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER): $(SHARED_OBJS) $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_CLIENT): $(SHARED_OBJS) $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


$(SHARED_DIR)/%.o: $(SHARED_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SERVER_DIR)/%.o: $(SERVER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CLIENT_DIR)/%.o: $(CLIENT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT) \
	      $(SHARED_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS)