fileHeaders = {};
fileHeaders.CAN    = {'R1','R2','R3','R4'};
fileHeaders.LEO    = {'t','k_pos','k_vel','k_acc','a_pos','a_vel','a_acc','e_pos','e_vel','e_acc','R10'};
fileHeaders.Markov = {'dt','t','R_d_T','R_T','R_Fx','R_w_l','R_t_l','R_w_p','R_t_p','R_i_T_e','R_u',...
                      'R_T_d','R_t_m','R_d_w_l','phase','L_t_m','L_t_l','L_u','L_T_r','L_T_d'};
fileHeaders.XSENS  = {'t','G_x_1','G_y_1','G_z_1','A_x_1','A_y_1','A_z_1',...
                          'G_x_2','G_y_2','G_z_2','A_x_2','A_y_2','A_z_2',...
                          'G_x_3','G_y_3','G_z_3','A_x_3','A_y_3','A_z_3',...
                          'G_x_4','G_y_4','G_z_4','A_x_4','A_y_4','A_z_4'};
fileHeaders.ATI    = {'t','Fx','F_y','F_z','Tx','T_y','T_z'};
fileHeaders.GAIT   = {'t','mode','S','prom'};

fileHeadersLatex = {};
fileHeadersLatex.CAN    = {'Raw_{1}','Raw_{2}','Raw_{3}','Raw_{4}'};
fileHeadersLatex.LEO    = {'t','\theta_{knee}','\omega_{knee}','\dot{\theta_{knee}}','\theta_{ankle}','\omega_{ankle}','\dot{\theta_{ankle}}',...
                           '\theta_{exo}','\omega_{exo}','\dot{\theta_{exo}}','Raw_{10}'};
fileHeadersLatex.Markov = {'\delta{t}','t','X_{1}','X_{2}','X_{3}','X_{4}','X_{5}','X_{6}','X_{7}','X_{8}','u_{r}',...
                         '\tau_{r}^{d}','\theta_{r}^{m}','\dot{\omega}_{r}{l}','\theta_{k}','\theta_{l}^{m}','\theta_{l}^{l}','u_{l}','\tau_{l}','\tau_{l}^{d}'};
fileHeadersLatex.XSENS  = {'t','\theta_{x}^{1}','\theta_{y}^{1}','\theta_{z}^{1}','\dot{\omega}_{x}^{1}','\dot{\omega}_{y}^{1}','\dot{\omega}_{z}^{1}',...
                               '\theta_{x}^{2}','\theta_{y}^{2}','\theta_{z}^{2}','\dot{\omega}_{x}^{2}','\dot{\omega}_{y}^{2}','\dot{\omega}_{z}^{2}',...
                               '\theta_{x}^{3}','\theta_{y}^{3}','\theta_{z}^{3}','\dot{\omega}_{x}^{3}','\dot{\omega}_{y}^{3}','\dot{\omega}_{z}^{3}',...
                               '\theta_{x}^{4}','\theta_{y}^{4}','\theta_{z}^{4}','\dot{\omega}_{x}^{4}','\dot{\omega}_{y}^{4}','\dot{\omega}_{z}^{4}'};
fileHeadersLatex.ATI    = {'t','F_{x}','F_{y}','F_{z}','T_{x}','T_{y}','T_{z}'};
fileHeadersLatex.GAIT   = {'t','\theta_{k}','S','mean'};

nameTest = {'Markov Free \tau^{} = 0'
            'Human free movements'
            'PID Free \tau^{} = 0'
            'PID Free \tau^{} = Kv*Bv'
            'Markov Gait \tau^{} = 0'
            'Markov Gait \tau^{} = Kv*Bv'
            'PID Gait \tau^{} = 0'
            'PID Gait \tau^{} = Kv*Bv'};
nameControlTest = { "Markovian"
            "Markovian"
            "PID"
            "PID"
            "Markovian"
            "Markovian"
            "PID"
            "PID"
            ''};

nameSubplot = {'a) Torques'
               'b) Velocities'};
           
namesMarkovExtended = {};
namesMarkovExtended.R_T     = '(R) Torque do Rôbo ';
namesMarkovExtended.R_Fx    = '(R) Sensor de Torque Tx';
namesMarkovExtended.R_w_l   = '(R) Velocidade coroa';
namesMarkovExtended.R_t_l   = '(R) Ângulo coroa';
namesMarkovExtended.R_t_m   = '(R) Ângulo motor';
namesMarkovExtended.R_w_p   = '(R) Velocidade da pessona'; 
namesMarkovExtended.R_t_p   = '(R) Ângulo da pessona';
namesMarkovExtended.R_i_T_e = '(R) Integral do erro de Torque';
namesMarkovExtended.R_u     = '(R) Sinal de controle';
namesMarkovExtended.R_T_d   = '(R) Torque desejado';
namesMarkovExtended.R_T_i   = '(R) Torque Interação';

times_fases = [0 .25 .35 .6 .8 1];
eventos_fases = {"HS",'TS','HO','TO','MSw','HS'};
