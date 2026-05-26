function data = init(N,varargin)
    clc
    if(N ~= 0)
        global DATA1 DATA2
        if(~isempty(DATA1)) 

            if(N==1)
                data = DATA1;
            else
                data = DATA2;
            end
            
            Ts200 = 1/200;    
            for d_idx = 1:numel(data)
                
                if mean(data{d_idx}.Markov.R_w_l) == 0
                    data{d_idx}.Markov.R_w_l = ([data{d_idx}.Markov.R_t_l;0;0] - [0;0;data{d_idx}.Markov.R_t_l])/(2*Ts200); 
                    data{d_idx}.Markov.R_w_l = data{d_idx}.Markov.R_w_l(1:end-2,:);
                end
                
                data{d_idx}.Markov.L_w_l = ([data{d_idx}.Markov.L_t_l;0;0] - [0;0;data{d_idx}.Markov.L_t_l])/(2*Ts200); 
                data{d_idx}.Markov.L_w_l = data{d_idx}.Markov.L_w_l(1:end-2,:);
                
                data{d_idx}.Markov.R_w_l = movmean(data{d_idx}.Markov.R_w_l,10);
                data{d_idx}.Markov.L_w_l = movmean(data{d_idx}.Markov.L_w_l,10);
                
                data{d_idx}.Markov.R_T_i = (data{d_idx}.Markov.R_T - data{d_idx}.Markov.R_Fx);
                data{d_idx}.Markov.R_T_i = movmean(data{d_idx}.Markov.R_T_i,10);
                
                data{d_idx}.Markov.R_w_p = -movmean(data{d_idx}.Markov.R_w_p,10);
                                
                d_phase = (diff([0;data{d_idx}.Markov.phase])~=0) .* data{d_idx}.Markov.phase;
                idx_phase = [find(d_phase ~= 0) ; length(d_phase)];
    
                data{d_idx}.Markov.idx_phase = idx_phase;
                namesMarkov = fieldnames(data{d_idx}.Markov);
                d_phase = {};
                for p_idx = 1:5
                    d_phase{p_idx}.count = 0;
                    for n_idx = 1:numel(namesMarkov)
                        if isnumeric(data{d_idx}.Markov.(namesMarkov{n_idx}))
                            d_phase{p_idx}.(namesMarkov{n_idx}) = {};
                        end
                    end

                end

                for p_idx = 1:(numel(idx_phase)-1)

                    t1 = idx_phase(p_idx);
                    t2 = idx_phase(p_idx+1);
                    n_phase = data{d_idx}.Markov.phase(t1);

                    d_phase{n_phase}.count = d_phase{n_phase}.count + 1;
                    c = d_phase{n_phase}.count;

                    for n_idx = 1:numel(namesMarkov)
                        if(isequal((namesMarkov{n_idx}),'idx_phase'))
                            continue;
                        end
                        if isnumeric(data{d_idx}.Markov.(namesMarkov{n_idx}))
                            d = data{d_idx}.Markov.(namesMarkov{n_idx});
                            d_phase{n_phase}.(namesMarkov{n_idx}){c} = d(t1:t2,:);
                        end
                    end

                end
                
                data{d_idx}.d_phase = d_phase;
                
                %-------------------------------------------------------------------------
            end

            return;
        end
    else
        N=1;
    end
    
    clearvars -except N varargin
    
    global DATA1 DATA2
    
    path_data = '../../CDC_LOGS/';
    formatFolder = 'T%d%s/';

    fileNames = {};
    fileNames.CAN    = 'last_data_EposEXO CAN 1.dat';
    fileNames.LEO    = 'last_data_XSens Leo 1.dat';
    fileNames.Markov = 'last_data_MarkovMao 1.dat';
    fileNames.XSENS  = 'last_data_XSens Read 1.dat';
    fileNames.ATI    = 'last_data_ATIMX 1.dat';
    fileNames.GAIT   = 'last_data_XSens  Gait Phase 1.dat';
    names = fieldnames(fileNames);
    payload = {'' '_1'};
    
    headers();
    
    if nargin >= 1
        t_lims = varargin{1};
        t_lims.t1 = t_lims.t1 * 200 + 1;
        t_lims.t2 = t_lims.t2 * 200 ;
    else
        t_lims = struct('t1',1,'t2',0);
    end
    
    
    
    forTest = 1:8;
    DATA1 = {};
    
    for t_idx = forTest

        DATA1{t_idx} = {};
        nameFolder = sprintf(formatFolder,t_idx,'');
        for n_idx = 1:numel(names)
            name = names{n_idx};

            DATA1{t_idx}.(name).('RAW') = load([path_data nameFolder fileNames.(name)]);
            DATA1{t_idx}.(name).('RAW') = DATA1{t_idx}.(name).('RAW')(t_lims.t1:end-t_lims.t2,:);
            str_header = fileHeaders.(name);
            for h_idx = 1:numel(str_header)
                DATA1{t_idx}.(name).(str_header{h_idx}) = DATA1{t_idx}.(name).('RAW')(:,h_idx);
                DATA1{t_idx}.(name).(['Latex_' str_header{h_idx}]) = fileHeadersLatex.(name){h_idx};
            end
            
        end
    end
    
    DATA2 = DATA1;
   
    for t_idx_ = forTest
        t_idx = t_idx_;
        nameFolder = sprintf(formatFolder,t_idx_,'_1');
        for n_idx = 1:numel(names)
            name = names{n_idx};
            try
                DATA2{t_idx}.(name).('RAW') = load([path_data nameFolder fileNames.(name)]);
                DATA2{t_idx}.(name).('RAW') = DATA2{t_idx}.(name).('RAW')(t_lims.t1:end-t_lims.t2,:);
            catch
                break;
            end
            
            str_header = fileHeaders.(name);
            for h_idx = 1:numel(str_header)
                DATA2{t_idx}.(name).(str_header{h_idx}) = DATA2{t_idx}.(name).('RAW')(:,h_idx);
            end

        end
    end
    
    data = init(N,varargin);
    
   
